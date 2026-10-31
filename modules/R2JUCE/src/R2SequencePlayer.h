#pragma once
// Plays a SequenceFile into a MIDI sink from an audio callback. Transport
// commands (load, play, pause, stop) may come from any thread; process()
// runs on the audio thread, which owns the playback clock.
//
// Events go out in file order at 1 ms steps of the audio clock, so a large
// host block cannot quantise note starts (process()), or for a synth that
// takes sample-stamped MIDI, each at its own sample of the block
// (processStamped()). Loading a sequence or stopping
// releases every note and resets the controllers; pausing releases the
// notes. At the end of the sequence (plus one bar) playback rewinds and
// stops, and takeFinished() reports it once.

#include "R2SequenceFile.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>

namespace r2juce
{
class SequencePlayer
{
public:
    // Any thread. The sequence must outlive its playback; nullptr unloads.
    void load (const SequenceFile* sequence) noexcept
    {
        playing.store (false, std::memory_order_release);
        finished.store (false, std::memory_order_release);
        positionForUi.store (0.0, std::memory_order_release);
        loaded.store (sequence != nullptr, std::memory_order_release);
        pending.store (sequence, std::memory_order_release);
        commands.fetch_or (loadCommand, std::memory_order_release);
    }

    void play() noexcept
    {
        if (hasSequence())
            playing.store (true, std::memory_order_release);
    }

    void pause() noexcept
    {
        finished.store (false, std::memory_order_release);
        playing.store (false, std::memory_order_release);
        commands.fetch_or (pauseCommand, std::memory_order_release);
    }

    // Also when nothing is loaded: releases notes and resets controllers.
    void stop() noexcept
    {
        finished.store (false, std::memory_order_release);
        playing.store (false, std::memory_order_release);
        positionForUi.store (0.0, std::memory_order_release);
        commands.fetch_or (stopCommand, std::memory_order_release);
    }

    bool hasSequence() const noexcept { return loaded.load (std::memory_order_acquire); }
    bool isPlaying() const noexcept { return playing.load (std::memory_order_acquire); }
    // As of the last process() call.
    double getPositionSeconds() const noexcept { return positionForUi.load (std::memory_order_acquire); }
    // True once after the sequence has played to its end.
    bool takeFinished() noexcept { return finished.exchange (false, std::memory_order_acq_rel); }

    // Audio thread. Runs `numSamples` of audio: song events go to
    // send (const uint8_t* data, int size), and render (int offset, int count)
    // renders the audio between them. other (int rendered) is visited at each
    // step after the song's events, for the caller's own events (host MIDI);
    // it returns the position of its next event, or numSamples when none.
    template <typename Send, typename Render, typename Other>
    void process (int numSamples, double sampleRate, Send&& send, Render&& render, Other&& other) noexcept
    {
        applyCommands (send);
        const bool playingNow = playing.load (std::memory_order_relaxed) && active != nullptr;
        // Integer sample counts make a step just under 1 ms at common rates
        // (44 samples at 44.1 kHz).
        const int step = playingNow && sampleRate > 0.0 ? std::max (1, static_cast<int> (sampleRate * 0.001))
                                                        : numSamples;
        int rendered = 0;
        while (rendered < numSamples)
        {
            if (playingNow) dispatch (send);
            int next = std::min (numSamples, other (rendered));
            if (playingNow) next = std::min (next, rendered + step);
            next = std::max (next, rendered + 1);
            render (rendered, next - rendered);
            if (playingNow && sampleRate > 0.0)
                position += static_cast<double> (next - rendered) / sampleRate;
            rendered = next;
        }
        // Events exactly at the end of the block go out now and reach the
        // synth when the next block starts.
        if (playingNow) dispatch (send);
        other (rendered);

        if (playingNow && position > active->totalSeconds())
            finish();
        positionForUi.store (position, std::memory_order_release);
    }

    // Audio thread, for a synth that takes sample-stamped MIDI for a whole
    // block: every song event due in these `numSamples` goes to
    // send (const uint8_t* data, int size, int sampleOffset) at its own
    // sample, in file order; the caller then renders the block in one go.
    // other() sends the caller's own events (host MIDI) after the transport's
    // notes-off and resets and before the song's, for a synth whose clock
    // never goes back.
    template <typename SendAt, typename Other>
    void processStamped (int numSamples, double sampleRate, SendAt&& send, Other&& other) noexcept
    {
        auto sendNow = [&send] (const std::uint8_t* data, int size) { send (data, size, 0); };
        applyCommands (sendNow);
        other();
        if (playing.load (std::memory_order_relaxed) && active != nullptr && sampleRate > 0.0)
        {
            const auto blockEnd = position + numSamples / sampleRate;
            const auto& events = active->events;
            while (next < events.size() && events[next].seconds < blockEnd)
            {
                const auto& event = events[next++];
                const auto offset = std::clamp (static_cast<int> ((event.seconds - position) * sampleRate),
                                                0, std::max (0, numSamples - 1));
                send (event.bytes.data(), static_cast<int> (event.bytes.size()), offset);
            }
            position = blockEnd;
            if (position > active->totalSeconds())
                finish();
        }
        positionForUi.store (position, std::memory_order_release);
    }

    // Release pedal-held keys, then all notes, then all sound: CC 123 alone
    // can leave a long release audible, and these are transport actions.
    template <typename Send>
    static void sendAllNotesOff (Send&& send) noexcept
    {
        for (int channel = 0; channel < 16; ++channel)
        {
            sendController (send, channel, 64);
            sendController (send, channel, 66);
        }
        for (const std::uint8_t controller : { std::uint8_t (123), std::uint8_t (120) })
            for (int channel = 0; channel < 16; ++channel)
                sendController (send, channel, controller);
    }

    template <typename Send>
    static void sendResetAllControllers (Send&& send) noexcept
    {
        for (int channel = 0; channel < 16; ++channel)
            sendController (send, channel, 121);
    }

private:
    enum : std::uint32_t { loadCommand = 1u << 0, pauseCommand = 1u << 1, stopCommand = 1u << 2 };

    template <typename Send>
    static void sendController (Send& send, int channel, std::uint8_t controller) noexcept
    {
        const std::array<std::uint8_t, 3> message { static_cast<std::uint8_t> (0xb0 | channel), controller, 0 };
        send (message.data(), static_cast<int> (message.size()));
    }

    template <typename Send>
    void applyCommands (Send& send) noexcept
    {
        const auto pendingCommands = commands.exchange (0, std::memory_order_acq_rel);
        if ((pendingCommands & loadCommand) != 0)
        {
            // A new sequence must not inherit notes or controllers from the last.
            active = pending.load (std::memory_order_acquire);
            position = 0.0;
            next = 0;
            sendAllNotesOff (send);
            sendResetAllControllers (send);
        }
        if ((pendingCommands & pauseCommand) != 0)
            sendAllNotesOff (send);
        if ((pendingCommands & stopCommand) != 0)
        {
            sendAllNotesOff (send);
            sendResetAllControllers (send);
            position = 0.0;
            next = 0;
        }
    }

    // Natural completion rewinds like Stop, so the next Play starts at the first event.
    void finish() noexcept
    {
        playing.store (false, std::memory_order_release);
        finished.store (true, std::memory_order_release);
        position = 0.0;
        next = 0;
    }

    template <typename Send>
    void dispatch (Send& send) noexcept
    {
        // File order is preserved exactly; nothing here sorts or merges events.
        while (next < active->events.size() && active->events[next].seconds <= position)
        {
            const auto& event = active->events[next++];
            send (event.bytes.data(), static_cast<int> (event.bytes.size()));
        }
    }

    std::atomic<const SequenceFile*> pending { nullptr };
    std::atomic<std::uint32_t> commands { 0 };
    std::atomic<bool> loaded { false }, playing { false }, finished { false };
    std::atomic<double> positionForUi { 0.0 };

    // Audio thread only.
    const SequenceFile* active = nullptr;
    double position = 0.0;
    std::size_t next = 0;
};
} // namespace r2juce
