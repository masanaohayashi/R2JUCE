#pragma once
// Shift-JIS text (RCP / WRD / SMF text from Japanese software) as a juce::String, with the platform's own
// converter. Include it in a .cpp, not the module header: it pulls in CoreFoundation / windows.h.

#include <juce_core/juce_core.h>

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#if JUCE_MAC || JUCE_IOS
 #include <CoreFoundation/CoreFoundation.h>
#elif JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
#endif

namespace r2juce
{
// ASCII, and UTF-8 where the platform has no converter, comes through as it is.
inline juce::String fromShiftJis (const std::string& bytes)
{
    if (bytes.empty())
        return {};

#if JUCE_MAC || JUCE_IOS
    if (auto* converted = CFStringCreateWithBytes (kCFAllocatorDefault,
                                                    reinterpret_cast<const UInt8*> (bytes.data()),
                                                    static_cast<CFIndex> (bytes.size()),
                                                    kCFStringEncodingShiftJIS, false))
    {
        const auto length = CFStringGetLength (converted);
        const auto maximumBytes = CFStringGetMaximumSizeForEncoding (length, kCFStringEncodingUTF8) + 1;
        std::string utf8 (static_cast<std::size_t> (std::max<CFIndex> (1, maximumBytes)), '\0');
        const auto success = CFStringGetCString (converted, utf8.data(), static_cast<CFIndex> (utf8.size()),
                                                 kCFStringEncodingUTF8);
        CFRelease (converted);
        if (success)
            return juce::String::fromUTF8 (utf8.c_str());
    }
#elif JUCE_WINDOWS
    const auto byteCount = static_cast<int> (std::min<std::size_t> (
        bytes.size(), static_cast<std::size_t> (std::numeric_limits<int>::max())));
    const auto wideLength = MultiByteToWideChar (932, 0, bytes.data(), byteCount, nullptr, 0);
    if (wideLength > 0)
    {
        std::vector<wchar_t> wide (static_cast<std::size_t> (wideLength) + 1, L'\0');
        if (MultiByteToWideChar (932, 0, bytes.data(), byteCount, wide.data(), wideLength) > 0)
            return juce::String (juce::CharPointer_UTF16 (
                reinterpret_cast<const juce::CharPointer_UTF16::CharType*> (wide.data())),
                static_cast<std::size_t> (wideLength));
    }
#endif

    return juce::String::fromUTF8 (bytes.data(), static_cast<int> (bytes.size()));
}
} // namespace r2juce
