#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace r2juce { struct SequenceFile; }

namespace r2juce::rcp
{
/** Returns true when the buffer starts with the RCP v2 header. */
bool isRcpV2 (const std::vector<std::uint8_t>& data) noexcept;

/**
    Parses an RCP v2 sequence and expands it into the SequenceFile event
    list.  Plain C++, free of JUCE, so it can be built and tested anywhere.
*/
bool loadRcpV2 (const std::vector<std::uint8_t>& data,
                r2juce::SequenceFile& destination,
                std::string& error,
                const std::string& sourcePath = {});
}
