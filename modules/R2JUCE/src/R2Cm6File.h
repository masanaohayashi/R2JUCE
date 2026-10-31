#pragma once
#include "R2SequenceFile.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

// Recomposer CM6 layout: LA system/temporary/patch/timbre memory, followed
// by CM-32P temporary/patch/system memory. Addresses are Roland base-128.
// Format reference: https://github.com/ValleyBell/MidiConverters/blob/master/rcp2mid.c
namespace r2juce::cm6
{
inline bool isCm6 (const std::vector<uint8_t>& bytes)
{
    return bytes.size() >= 0x20 && std::memcmp (bytes.data(), "COME ON MUSIC", 14) == 0
        && bytes[14] == 0 && bytes[15] == 0 && bytes[16] == 'R' && bytes[17] == ' ';
}

inline void prepend (const std::vector<uint8_t>& bytes, SequenceFile& song)
{
    if (! isCm6 (bytes) || bytes.size() < 0x5849)
        throw std::runtime_error ("Invalid or truncated CM6 control file");
    struct Region { std::size_t offset, size; unsigned address; bool pcm; };
    constexpr Region regions[] {
        { 0x0080, 0x0017, 0x100000, false },
        { 0x00a0, 0x0090, 0x030000, false },
        { 0x0130, 0x0154, 0x030110, false },
        { 0x0284, 0x07b0, 0x040000, false },
        { 0x0a34, 0x0400, 0x050000, false },
        { 0x0e34, 0x4000, 0x080000, false },
        { 0x4e34, 0x007e, 0x500000, true },
        { 0x4eb2, 0x0980, 0x510000, true },
        { 0x5832, 0x0011, 0x520000, true }
    };
    std::vector<SequenceEvent> setup;
    double seconds = 0.0;
    for (const auto& region : regions)
    {
        if (region.pcm && bytes[0x1a] == 0) continue; // MT-32-only CM6
        const unsigned base = ((region.address >> 16) & 127) * 16384
                            + ((region.address >> 8) & 127) * 128 + (region.address & 127);
        for (std::size_t pos = 0; pos < region.size;)
        {
            const auto count = std::min (std::size_t (256), region.size - pos);
            const auto address = base + static_cast<unsigned> (pos);
            std::vector<uint8_t> message { 0xf0, 0x41, 0x10, 0x16, 0x12,
                static_cast<uint8_t> ((address >> 14) & 127),
                static_cast<uint8_t> ((address >> 7) & 127),
                static_cast<uint8_t> (address & 127) };
            unsigned checksum = message[5] + message[6] + message[7];
            for (std::size_t i = 0; i < count; ++i)
            {
                const auto value = bytes[region.offset + pos + i];
                if (value > 127) throw std::runtime_error ("CM6 contains invalid SysEx data");
                message.push_back (value);
                checksum += value;
            }
            message.push_back (static_cast<uint8_t> ((128 - (checksum & 127)) & 127));
            message.push_back (0xf7);
            const auto transmissionTime = static_cast<double> (message.size()) / 3125.0;
            setup.push_back ({ seconds, std::move (message) });
            // DIN MIDI transmission time plus receiver processing time. The
            // whole setup must finish before the first musical event, also on replay.
            seconds += transmissionTime + 0.02;
            pos += count;
        }
    }
    for (auto& event : song.events) event.seconds += seconds;
    for (auto& frame : song.wrdFrames) frame.seconds += seconds;
    song.songEndSeconds += seconds;
    setup.insert (setup.end(), std::make_move_iterator (song.events.begin()),
                  std::make_move_iterator (song.events.end()));
    song.events = std::move (setup);
}
}
