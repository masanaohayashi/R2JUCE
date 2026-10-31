#pragma once
// C++ adaptation of ExtractMDF Sources/LZMCore/MDFArchive.swift.
// https://github.com/masanaohayashi/ExtractMDF
/*
MIT License

Copyright (c) 2026 Masanao Takeuchi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace r2juce::mdf
{
constexpr std::size_t sizeLimit = 256u * 1024 * 1024;
struct Entry
{
    std::string name; // Original Shift-JIS bytes, used only for companion matching.
    std::vector<uint8_t> data;
};

inline std::vector<Entry> decode (const std::vector<uint8_t>& bytes)
{
    const auto fail = [] { throw std::runtime_error ("Invalid or damaged LZM MDF0 archive"); };
    if (bytes.size() < 160 || bytes.size() > sizeLimit
        || std::memcmp (bytes.data(), "MDF0\r\n", 6) != 0) fail();
    const auto integer = [&] (std::size_t at) -> std::size_t
    {
        std::size_t value = 0;
        for (int i = 0; i < 4; ++i) value = (value << 8) | bytes[at + i];
        return value;
    };
    const auto count = integer (76);
    if (count == 0 || count > 4096) fail();
    std::vector<Entry> entries;
    std::size_t offset = 160, total = 0;
    for (std::size_t index = 0; index < count; ++index)
    {
        if (offset > bytes.size() || bytes.size() - offset < 48) fail();
        const auto compressed = integer (offset), expanded = integer (offset + 44);
        if (compressed == 0 || compressed > bytes.size() - offset - 48
            || expanded > sizeLimit || expanded > 2 * sizeLimit - total) fail();
        total += expanded;
        std::string name;
        for (std::size_t i = offset + 16; i < offset + 44 && bytes[i] != 0; ++i)
            name += static_cast<char> (bytes[i]);
        if (name.empty()) fail();
        std::size_t position = offset + 48;
        const auto end = position + compressed;
        unsigned flags = 0, remaining = 0;
        const auto byte = [&] () -> unsigned
        {
            if (position == end) fail();
            return bytes[position++];
        };
        const auto bit = [&] () -> bool
        {
            if (remaining == 0) { flags = byte(); remaining = 8; }
            const bool value = (flags & 0x80) != 0;
            flags <<= 1;
            --remaining;
            return value;
        };
        std::vector<uint8_t> output;
        output.reserve (expanded);
        for (;;)
        {
            if (bit())
            {
                if (output.size() == expanded) fail();
                output.push_back (static_cast<uint8_t> (byte()));
                continue;
            }
            std::size_t distance, length;
            if (bit())
            {
                const auto high = byte();
                const auto word = (high << 8) | byte();
                distance = 8192 - (word >> 3);
                const auto code = word & 7;
                if (code == 0)
                {
                    const auto extra = byte();
                    if (extra == 0) break;
                    length = extra + 2;
                }
                else length = code + 2;
            }
            else
            {
                const auto high = bit() ? 2u : 0u;
                const auto low = bit() ? 1u : 0u;
                length = high + low + 2;
                distance = 256 - byte();
            }
            if (distance > output.size() || length > expanded - output.size()) fail();
            for (std::size_t i = 0; i < length; ++i)
                output.push_back (output[output.size() - distance]);
        }
        if (output.size() != expanded) fail();
        entries.push_back ({ std::move (name), std::move (output) });
        offset = end;
    }
    return entries;
}
}
