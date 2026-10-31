#include "R2SequenceFile.h"
#include "R2RcpFile.h"
#include "R2MdfArchive.h"
#include "R2Cm6File.h"
#include <atomic>
#include <chrono>
#include <fstream>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>

namespace r2juce
{
namespace sequence_detail
{
namespace
{
template <typename String>
std::string foldedName (const String& name)
{
    std::string result (name.begin(), name.end());
    for (auto& ch : result) if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
    return result;
}

// A UTF-8 path as std::filesystem sees it, in C++17 and C++20 alike.
std::filesystem::path pathFromUtf8 (const std::string& utf8)
{
   #if defined (__cpp_char8_t)
    return std::filesystem::path (std::u8string (utf8.begin(), utf8.end()));
   #else
    return std::filesystem::u8path (utf8);
   #endif
}

std::vector<uint8_t> readControlFile (const std::filesystem::path& path)
{
    if (std::filesystem::file_size (path) > mdf::sizeLimit)
        throw std::runtime_error ("CM6 file exceeds size limit");
    std::ifstream input (path, std::ios::binary);
    if (! input) throw std::runtime_error ("Cannot read companion CM6");
    std::vector<uint8_t> result ((std::istreambuf_iterator<char> (input)), {});
    if (input.bad()) throw std::runtime_error ("Cannot read companion CM6");
    return result;
}

struct Reader
{
    const uint8_t* p;
    const uint8_t* end;

    bool ok() const { return p < end; }
    uint8_t u8() { return p < end ? *p++ : 0; }
    uint32_t varlen()
    {
        uint32_t v = 0;
        for (int i = 0; i < 4; ++i)
        {
            const uint8_t c = u8();
            v = (v << 7) | (c & 0x7f);
            if (! (c & 0x80)) break;
        }
        return v;
    }
};

struct RawEvent
{
    uint32_t tick;
    uint32_t order;
    std::vector<uint8_t> bytes;
    uint32_t tempo;
    uint8_t numerator, denominator;
};
}
}
using namespace sequence_detail;

bool SequenceFile::load (const std::string& path, std::string& error, bool loadWrd, bool loadCm6)
{
    clear();
    error.clear();

   #if defined (_WIN32)
    // JUCE supplies UTF-8 paths; the narrow Windows CRT uses the system code page.
    const std::filesystem::path nativePath (std::u8string (path.begin(), path.end()));
    std::FILE* file = _wfopen (nativePath.c_str(), L"rb");
   #else
    std::FILE* file = std::fopen (path.c_str(), "rb");
   #endif
    if (file == nullptr) { error = "Cannot open: " + path; return false; }

    std::fseek (file, 0, SEEK_END);
    const long size = std::ftell (file);
    std::rewind (file);
    if (size < 0 || static_cast<unsigned long> (size) > mdf::sizeLimit)
    { std::fclose (file); error = "Sequence file exceeds 256 MiB limit"; return false; }
    std::vector<uint8_t> data (static_cast<size_t> (std::max (0L, size)));
    const bool read = ! data.empty() && std::fread (data.data(), 1, data.size(), file) == data.size();
    std::fclose (file);
    if (! read) { error = "Cannot load: " + path; return false; }

    if (data.size() >= 4 && std::memcmp (data.data(), "MDF0", 4) == 0)
    {
        // Stage the selected sequence and its companions under fixed names. Archive
        // names never become paths; no files are written beside the user's MDF.
        struct TemporaryDirectory
        {
            std::filesystem::path path;
            ~TemporaryDirectory() { std::error_code ec; if (! path.empty()) std::filesystem::remove_all (path, ec); }
        } temporary;
        try
        {
            const auto entries = mdf::decode (data);
            const auto song = std::find_if (entries.begin(), entries.end(), [] (const auto& entry)
            {
                return rcp::isRcpV2 (entry.data)
                    || (entry.data.size() >= 14 && std::memcmp (entry.data.data(), "MThd", 4) == 0);
            });
            if (song == entries.end())
            { error = "MDF contains no supported MIDI or RCP v2 sequence"; return false; }
            // The clock and a counter name it: std::random_device may have no entropy to read
            // (an Emscripten AudioWorklet throws), and a name taken just moves on to the next.
            static std::atomic<unsigned> counter { 0 };
            const auto base = static_cast<unsigned long long> (std::chrono::system_clock::now().time_since_epoch().count());
            for (int attempt = 0; attempt < 32 && temporary.path.empty(); ++attempt)
            {
                auto candidate = std::filesystem::temp_directory_path()
                               / ("r2juce-mdf-" + std::to_string (base) + "-" + std::to_string (counter++));
                if (std::filesystem::create_directory (candidate)) temporary.path = std::move (candidate);
            }
            if (temporary.path.empty()) throw std::runtime_error ("Cannot create MDF temporary directory");
            const auto write = [&] (const char* name, const std::vector<uint8_t>& bytes)
            {
                std::ofstream output (temporary.path / name, std::ios::binary);
                output.write (reinterpret_cast<const char*> (bytes.data()), static_cast<std::streamsize> (bytes.size()));
                output.close();
                if (! output) throw std::runtime_error ("Cannot stage MDF sequence");
            };
            write ("song.mid", song->data);
            const auto foldedStem = [] (std::string name)
            {
                name = name.substr (0, name.find_last_of ('.'));
                for (auto& ch : name) if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                return name;
            };
            if (loadWrd)
                for (const auto& entry : entries)
                {
                    const auto dot = entry.name.find_last_of ('.');
                    if (dot == std::string::npos) continue;
                    auto extension = entry.name.substr (dot);
                    for (auto& ch : extension) if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                    if (extension == ".wrd" && foldedStem (entry.name) == foldedStem (song->name))
                    { write ("song.wrd", entry.data); break; }
                }
            // Prefer the RCP header's control filename, then the matching stem;
            // a lone CM6 is also unambiguous in archives with renamed entries.
            const mdf::Entry* control = nullptr;
            if (loadCm6)
            {
                std::string referencedCm6;
                if (rcp::isRcpV2 (song->data) && song->data.size() >= 0x1d6)
                {
                    for (std::size_t i = 0x1c6; i < 0x1d6 && song->data[i] != 0; ++i)
                        referencedCm6 += static_cast<char> (song->data[i]);
                    while (! referencedCm6.empty() && referencedCm6.back() == ' ') referencedCm6.pop_back();
                }
                const mdf::Entry* onlyControl = nullptr;
                std::size_t controls = 0;
                for (const auto& entry : entries)
                {
                    const auto dot = entry.name.find_last_of ('.');
                    if (dot == std::string::npos || foldedName (entry.name.substr (dot)) != ".cm6") continue;
                    ++controls;
                    onlyControl = &entry;
                    if (! referencedCm6.empty() && foldedName (entry.name) == foldedName (referencedCm6))
                    { control = &entry; break; }
                    if (foldedStem (entry.name) == foldedStem (song->name)) control = &entry;
                }
                if (control == nullptr && controls == 1) control = onlyControl;
                // The recursive RCP load discovers this under its synthetic stem.
                if (control != nullptr && rcp::isRcpV2 (song->data)) write ("song.cm6", control->data);
            }
            const auto utf8 = (temporary.path / "song.mid").u8string();
            const bool loaded = load (std::string (utf8.begin(), utf8.end()), error, loadWrd, loadCm6);
            if (! wrdFilePath.empty()) wrdFilePath = path;
            if (loaded && control != nullptr && ! rcp::isRcpV2 (song->data))
                cm6::prepend (control->data, *this);
            return loaded;
        }
        catch (const std::exception& exception)
        { clear(); error = exception.what(); return false; }
    }

    try
    {
        if (loadCm6 && cm6::isCm6 (data))
        {
            cm6::prepend (data, *this);
            return true;
        }
        if (rcp::isRcpV2 (data))
        {
            if (! rcp::loadRcpV2 (data, *this, error, loadWrd ? path : std::string {}))
                return false;
            if (! loadCm6)
                return true;
            // Same-stem CM6 works independently of WRD and of the extension case.
            const auto source = pathFromUtf8 (path);
            auto companionName = source.filename();
            companionName.replace_extension (".cm6");
            const auto wanted = foldedName (companionName.u8string());
            const auto folder = source.has_parent_path() ? source.parent_path() : std::filesystem::path (".");
            for (const auto& entry : std::filesystem::directory_iterator (folder))
            {
                if (entry.is_regular_file() && foldedName (entry.path().filename().u8string()) == wanted)
                {
                    cm6::prepend (readControlFile (entry.path()), *this);
                    break;
                }
            }
            return true;
        }
    }
    catch (const std::exception& exception)
    { clear(); error = exception.what(); return false; }

    if (data.size() < 14 || std::memcmp (data.data(), "MThd", 4) != 0)
    { error = "Not as standard MIDI file"; return false; }

    const uint16_t trackCount = static_cast<uint16_t> ((data[10] << 8) | data[11]);
    const uint16_t division   = static_cast<uint16_t> ((data[12] << 8) | data[13]);
    if (division == 0 || (division & 0x8000) != 0)
    { error = "SMPTE is not supported"; return false; }

    std::vector<RawEvent> raw;
    size_t pos = 8 + ((data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7]);
    uint32_t order = 0;

    for (uint16_t track = 0; track < trackCount && pos + 8 <= data.size(); ++track)
    {
        const uint32_t length = (data[pos+4] << 24) | (data[pos+5] << 16) | (data[pos+6] << 8) | data[pos+7];
        Reader r { data.data() + pos + 8, data.data() + std::min (data.size(), pos + 8 + length) };
        pos += 8 + length;

        uint32_t tick = 0;
        uint8_t running = 0;

        while (r.ok())
        {
            tick += r.varlen();
            uint8_t status = *r.p;

            if (status & 0x80) { r.u8(); if (status < 0xf0) running = status; }
            else               { status = running; if (status == 0) break; }

            RawEvent e {};
            e.tick = tick;
            e.order = order++;

            if (status == 0xff)
            {
                const uint8_t type = r.u8();
                const uint32_t len = r.varlen();
                const uint8_t* body = r.p;
                r.p = std::min (r.end, r.p + len);

                if (type == 0x51 && len == 3) e.tempo = (body[0] << 16) | (body[1] << 8) | body[2];
                else if (type == 0x58 && len >= 2) { e.numerator = body[0]; e.denominator = static_cast<uint8_t> (1 << body[1]); }
                else continue;
            }
            else if (status == 0xf0 || status == 0xf7)
            {
                const uint32_t len = r.varlen();
                const uint8_t* body = r.p;
                r.p = std::min (r.end, r.p + len);

                if (status == 0xf0) e.bytes.push_back (0xf0);
                e.bytes.insert (e.bytes.end(), body, body + len);
            }
            else
            {
                const int operands = ((status & 0xf0) == 0xc0 || (status & 0xf0) == 0xd0) ? 1 : 2;
                e.bytes.push_back (status);
                for (int i = 0; i < operands; ++i) e.bytes.push_back (r.u8());
            }

            raw.push_back (std::move (e));
        }
    }

    std::stable_sort (raw.begin(), raw.end(), [] (const RawEvent& a, const RawEvent& b)
    {
        return a.tick != b.tick ? a.tick < b.tick : a.order < b.order;
    });

    double seconds = 0.0, quarter = 0.5;
    uint32_t lastTick = 0;
    int numerator = 4, denominator = 4;

    for (const auto& e : raw)
    {
        seconds += (e.tick - lastTick) * quarter / division;
        lastTick = e.tick;

        if (e.tempo != 0) quarter = e.tempo / 1e6;
        if (e.numerator != 0) { numerator = e.numerator; denominator = e.denominator; }

        if (! e.bytes.empty())
        {
            events.push_back ({ seconds, e.bytes });
            songEndSeconds = seconds;
        }

        lastBarSeconds = quarter * numerator * 4.0 / denominator;
    }

    if (events.empty()) { error = "演奏イベントがありません"; return false; }
    return true;
}
} // namespace r2juce
