#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace r2juce
{
// Reusable dot-matrix LCD renderer, adapted from TX81Z CharacterLcdComponent.
// Message-thread API; owns display data only, with no audio/controller dependency.
class R2CharacterLcd final : public juce::Component, private juce::Timer,
    private juce::ComponentMovementWatcher
{
public:
    using CharacterCode = char32_t;

    struct Layout
    {
        int columns = 16, rows = 2;
        int glyphWidth = 5, glyphHeight = 8;
        float dotWidth = 1, dotHeight = 1;
        float dotGapX = 0.25f, dotGapY = 0.25f;
        float characterGap = 1.25f, lineGap = 3;
        juce::BorderSize<float> padding { 2 };
    };

    struct Palette
    {
        juce::Colour lit { 0xff5a8f05 }, unlit { 0x00000000 }, background { 0xff202020 };
    };

    struct Glyph { std::vector<std::uint8_t> dots; }; // Row-major, nonzero = lit.

    struct Cursor
    {
        enum class Style { hidden, underline, block };
        int column = 0, row = 0;
        Style style = Style::hidden;
        Style blinkOverlay = Style::hidden;
        int blinkPeriodMs = 1000;
    };


    enum class Scaling { fit, pixelAligned };
    R2CharacterLcd();
    bool setLayout(const Layout&);
    const Layout& getLayout() const noexcept { return layout_; }
    void setPalette(Palette);
    void setScaling(Scaling);
    // HD44780 A00 codes 16..255; requires 5x8 cells. Custom codes 0..15 survive.
    bool loadStandardCharacterSet();
    bool registerCharacter(CharacterCode, const Glyph&);
    // Convenience for 5x8 cells: low five bits of each row, bit 4 on the left.
    bool registerCharacterRows(CharacterCode, const std::array<std::uint8_t, 8>&);
    void removeCharacter(CharacterCode);
    void clearCharacterSet();
    void clear(CharacterCode fill = U' ');
    bool setCell(int column, int row, CharacterCode);
    void write(int column, int row, std::u32string_view);
    void setCursor(Cursor);
    void setDisplayEnabled(bool);
    // A dot turning on or off fades between the unlit and the lit colour over ms (as a liquid crystal's response;
    // 0 = at once, the default). The cursor's dots too.
    void setFadeTime(int ms);
    int getFadeTime() const noexcept { return fadeMs_; }
    // The fade's clock: the component's own timer calls it (~60 Hz while dots are moving); tests can call it.
    // True while some dot has not reached its state yet.
    bool advanceFade(double elapsedMs);
    juce::Point<float> getNaturalSize() const;
    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;
    void componentMovedOrResized(bool, bool) override {}
    void componentPeerChanged() override;
    void componentVisibilityChanged() override;
    void updateTimer();
    void changed();                       // the dots' states may have changed: repaint (and fade)
    bool dotLit(int column, int row, int x, int y) const;
    Layout layout_;
    Palette palette_;
    Scaling scaling_ = Scaling::pixelAligned;
    Cursor cursor_;
    bool enabled_ = true, blinkOn_ = true;
    std::vector<CharacterCode> cells_;
    std::unordered_map<CharacterCode, Glyph> glyphs_;
    int fadeMs_ = 0;
    std::vector<float> level_;            // each dot's brightness 0..1 while fading (row-major over the whole panel)
    bool fading_ = false;
    double lastTickMs_ = 0, blinkPhaseMs_ = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2CharacterLcd)
};
}
