#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace r2juce
{

/**
    A Standard MIDI File flattened into one time-ordered event list.

    Events that share a tick keep the order they have in the file.  RPN and NRPN
    are a state machine - a select (CC 101/100 or 99/98) followed by a data entry
    (CC 6/38) - so reordering a pair silently loses the setting.  Sequencers that
    sort or de-duplicate same-tick controllers are exactly how a pitch bend range
    goes missing, which is why this parser is deliberately order-preserving.
*/
struct SequenceEvent
{
    double seconds;
    std::vector<uint8_t> bytes;
};

/**
    One decoded MAG background image, as plain RGB data so that file loading
    stays independent of JUCE; a display makes its own image from it.
*/
struct WrdBackgroundImage
{
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgb;
    std::string filePath;
};

/**
    One complete text-screen snapshot from a companion .WRD file.

    WRD files are authored for an 80x25 MIMPI text screen.  The parser keeps
    the original bytes here because the files are normally Shift-JIS; a
    display converts them when it paints.
*/
struct WrdDisplayFrame
{
    static constexpr std::size_t columns = 80;
    static constexpr std::size_t rows = 25;

    double seconds = 0.0;
    std::array<std::string, rows> lines;
    std::array<std::uint8_t, rows> colours {};
    int backgroundIndex = -1;
};

struct SequenceFile
{
    std::vector<SequenceEvent> events;
    std::vector<WrdDisplayFrame> wrdFrames;
    std::vector<WrdBackgroundImage> wrdBackgrounds;
    std::string wrdFilePath;
    std::string wrdParseError;
    double songEndSeconds = 0.0;
    double lastBarSeconds = 2.0;

    /** Loads a UTF-8 path. Returns false and fills in `error` if no usable SMF/RCP v2 sequence is found (including inside LZM MDF0).
        With `loadCm6`, also a Recomposer CM6 control file for the MT-32 / CM-32L / CM-64: a .cm6 on its own, an RCP's
        same-named one, or one inside the MDF; its settings go out as SysEx before the first musical event. */
    bool load (const std::string& path, std::string& error, bool loadWrd = true, bool loadCm6 = false);

    /** The song plus one bar of tail, so releases and reverb are not cut off. */
    double totalSeconds() const { return songEndSeconds + lastBarSeconds; }

    void clear()
    {
        events.clear();
        wrdFrames.clear();
        wrdBackgrounds.clear();
        wrdFilePath.clear();
        wrdParseError.clear();
        songEndSeconds = 0.0;
        lastBarSeconds = 2.0;
    }
};
} // namespace r2juce
