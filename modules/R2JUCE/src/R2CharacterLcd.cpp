#include "R2CharacterLcd.h"
#include "R2Hd44780A00Font.h"
#include <algorithm>
#include <cmath>

namespace r2juce
{
namespace
{
float inkSize(int dots, float size, float gap) { return dots * size + (dots - 1) * gap; }
bool nonnegative(float n) { return std::isfinite(n) && n >= 0; }
}

R2CharacterLcd::R2CharacterLcd()
    : juce::ComponentMovementWatcher(this), cells_(32, U' ') {}

bool R2CharacterLcd::setLayout(const Layout& l)
{
    // Bound total drawable dots as well as individual dimensions before products.
    if (l.columns <= 0 || l.rows <= 0 || l.glyphWidth <= 0 || l.glyphHeight <= 0
        || l.columns > 4096 || l.rows > 4096 || l.glyphWidth > 256 || l.glyphHeight > 256
        || std::uint64_t(l.columns) * l.rows * l.glyphWidth * l.glyphHeight > 1048576
        || !nonnegative(l.dotWidth) || l.dotWidth == 0
        || !nonnegative(l.dotHeight) || l.dotHeight == 0
        || !nonnegative(l.dotGapX) || !nonnegative(l.dotGapY)
        || !nonnegative(l.characterGap) || !nonnegative(l.lineGap)
        || !nonnegative(l.padding.getLeft()) || !nonnegative(l.padding.getRight())
        || !nonnegative(l.padding.getTop()) || !nonnegative(l.padding.getBottom()))
        return false;
    const auto width = l.columns * inkSize(l.glyphWidth, l.dotWidth, l.dotGapX)
        + (l.columns - 1) * l.characterGap + l.padding.getLeft() + l.padding.getRight();
    const auto height = l.rows * inkSize(l.glyphHeight, l.dotHeight, l.dotGapY)
        + (l.rows - 1) * l.lineGap + l.padding.getTop() + l.padding.getBottom();
    if (!std::isfinite(width) || !std::isfinite(height)) return false;
    if (layout_.glyphWidth != l.glyphWidth || layout_.glyphHeight != l.glyphHeight)
        glyphs_.clear();
    layout_ = l;
    cells_.assign(static_cast<std::size_t>(l.columns) * l.rows, U' ');
    cursor_ = {};
    updateTimer();
    repaint();
    return true;
}

void R2CharacterLcd::setPalette(Palette p) { palette_ = p; repaint(); }
void R2CharacterLcd::setScaling(Scaling s) { scaling_ = s; repaint(); }
bool R2CharacterLcd::loadStandardCharacterSet()
{
    if (layout_.glyphWidth != 5 || layout_.glyphHeight != 8) return false;
    for (unsigned code = 16; code < 256; ++code)
    {
        Glyph glyph;
        glyph.dots.reserve(40);
        for (auto row : detail::hd44780A00Rows[code])
            for (int x = 4; x >= 0; --x)
                glyph.dots.push_back(static_cast<std::uint8_t>((row >> x) & 1U));
        glyphs_[static_cast<CharacterCode>(code)] = std::move(glyph);
    }
    repaint();
    return true;
}

bool R2CharacterLcd::registerCharacter(CharacterCode code, const Glyph& glyph)
{
    if (glyph.dots.size() != static_cast<std::size_t>(layout_.glyphWidth * layout_.glyphHeight))
        return false;
    const auto found = glyphs_.find(code);
    if (found != glyphs_.end() && found->second.dots == glyph.dots) return true;
    glyphs_[code] = glyph;
    repaint();
    return true;
}
bool R2CharacterLcd::registerCharacterRows(CharacterCode code, const std::array<std::uint8_t, 8>& rows)
{
    if (layout_.glyphWidth != 5 || layout_.glyphHeight != 8) return false;
    Glyph glyph;
    glyph.dots.reserve(40);
    for (auto row : rows)
        for (int x = 4; x >= 0; --x)
            glyph.dots.push_back(static_cast<std::uint8_t>((row >> x) & 1U));
    return registerCharacter(code, glyph);
}
void R2CharacterLcd::removeCharacter(CharacterCode c) { if (glyphs_.erase(c)) repaint(); }
void R2CharacterLcd::clearCharacterSet() { glyphs_.clear(); repaint(); }
void R2CharacterLcd::clear(CharacterCode c)
{
    if (std::all_of(cells_.begin(), cells_.end(), [c](auto v) { return v == c; })) return;
    std::fill(cells_.begin(), cells_.end(), c);
    repaint();
}
bool R2CharacterLcd::setCell(int column, int row, CharacterCode c)
{
    if (column < 0 || column >= layout_.columns || row < 0 || row >= layout_.rows) return false;
    auto& cell = cells_[static_cast<std::size_t>(row * layout_.columns + column)];
    if (cell != c) { cell = c; repaint(); }
    return true;
}
void R2CharacterLcd::write(int column, int row, std::u32string_view text)
{
    if (column < 0 || column >= layout_.columns || row < 0 || row >= layout_.rows) return;
    const auto count = std::min(text.size(), static_cast<std::size_t>(layout_.columns - column));
    for (std::size_t i = 0; i < count; ++i) setCell(column + static_cast<int>(i), row, text[i]);
}
void R2CharacterLcd::setCursor(Cursor c)
{
    c.blinkPeriodMs = std::max(2, c.blinkPeriodMs);
    if (cursor_.column == c.column && cursor_.row == c.row && cursor_.style == c.style
        && cursor_.blinkOverlay == c.blinkOverlay && cursor_.blinkPeriodMs == c.blinkPeriodMs)
        return;
    cursor_ = c;
    blinkOn_ = true;
    updateTimer();
    repaint();
}
void R2CharacterLcd::setDisplayEnabled(bool e)
{
    if (enabled_ == e) return;
    enabled_ = e;
    updateTimer();
    repaint();
}
// A parent can show/hide this LCD without changing its own visible flag.
void R2CharacterLcd::componentPeerChanged() { updateTimer(); }
void R2CharacterLcd::componentVisibilityChanged() { updateTimer(); }
void R2CharacterLcd::updateTimer()
{
    if (enabled_ && isShowing() && cursor_.blinkOverlay != Cursor::Style::hidden)
        startTimer(cursor_.blinkPeriodMs / 2);
    else stopTimer();
}
void R2CharacterLcd::timerCallback() { blinkOn_ = !blinkOn_; repaint(); }

juce::Point<float> R2CharacterLcd::getNaturalSize() const
{
    return {layout_.padding.getLeft() + layout_.padding.getRight()
        + layout_.columns * inkSize(layout_.glyphWidth, layout_.dotWidth, layout_.dotGapX)
        + (layout_.columns - 1) * layout_.characterGap,
        layout_.padding.getTop() + layout_.padding.getBottom()
        + layout_.rows * inkSize(layout_.glyphHeight, layout_.dotHeight, layout_.dotGapY)
        + (layout_.rows - 1) * layout_.lineGap};
}

void R2CharacterLcd::paint(juce::Graphics& g)
{
    g.fillAll(palette_.background);
    if (getWidth() <= 0 || getHeight() <= 0) return;
    const auto natural = getNaturalSize();
    const auto scale = std::min(getWidth() / natural.x, getHeight() / natural.y);
    const auto physical = g.getInternalContext().getPhysicalPixelScaleFactor();
    const bool aligned = scaling_ == Scaling::pixelAligned;
    const auto dimension = [=](float v) {
        if (!aligned || v == 0) return v * scale;
        return std::max(1.0f, std::floor(v * scale * physical)) / physical;
    };
    const auto dw = dimension(layout_.dotWidth), dh = dimension(layout_.dotHeight);
    const auto gx = dimension(layout_.dotGapX), gy = dimension(layout_.dotGapY);
    const auto cg = dimension(layout_.characterGap), lg = dimension(layout_.lineGap);
    const auto cw = inkSize(layout_.glyphWidth, dw, gx), ch = inkSize(layout_.glyphHeight, dh, gy);
    const auto left = dimension(layout_.padding.getLeft()), right = dimension(layout_.padding.getRight());
    const auto top = dimension(layout_.padding.getTop()), bottom = dimension(layout_.padding.getBottom());
    auto x0 = (getWidth() - (left + right + layout_.columns * cw + (layout_.columns - 1) * cg)) / 2 + left;
    auto y0 = (getHeight() - (top + bottom + layout_.rows * ch + (layout_.rows - 1) * lg)) / 2 + top;
    if (aligned) { x0 = std::round(x0 * physical) / physical; y0 = std::round(y0 * physical) / physical; }
    juce::RectangleList<float> on, off;
    for (int row = 0; row < layout_.rows; ++row)
        for (int column = 0; column < layout_.columns; ++column)
        {
            const auto glyph = glyphs_.find(cells_[static_cast<std::size_t>(row * layout_.columns + column)]);
            const bool cursorHere = cursor_.column == column && cursor_.row == row;
            for (int y = 0; y < layout_.glyphHeight; ++y)
                for (int x = 0; x < layout_.glyphWidth; ++x)
                {
                    const auto cursorDot = [this, y](Cursor::Style style) {
                        return style == Cursor::Style::block
                            || (style == Cursor::Style::underline && y == layout_.glyphHeight - 1);
                    };
                    const bool lit = enabled_ && ((glyph != glyphs_.end()
                        && glyph->second.dots[static_cast<std::size_t>(y * layout_.glyphWidth + x)] != 0)
                        || (cursorHere && (cursorDot(cursor_.style)
                            || (blinkOn_ && cursorDot(cursor_.blinkOverlay)))));
                    (lit ? on : off).addWithoutMerging({x0 + column * (cw + cg) + x * (dw + gx),
                        y0 + row * (ch + lg) + y * (dh + gy), dw, dh});
                }
        }
    g.setColour(palette_.unlit); g.fillRectList(off);
    g.setColour(palette_.lit); g.fillRectList(on);
}
}
