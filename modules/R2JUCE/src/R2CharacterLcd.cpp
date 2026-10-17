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
    changed();
    return true;
}

bool R2CharacterLcd::registerCharacter(CharacterCode code, const Glyph& glyph)
{
    if (glyph.dots.size() != static_cast<std::size_t>(layout_.glyphWidth * layout_.glyphHeight))
        return false;
    const auto found = glyphs_.find(code);
    if (found != glyphs_.end() && found->second.dots == glyph.dots) return true;
    glyphs_[code] = glyph;
    changed();
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
void R2CharacterLcd::removeCharacter(CharacterCode c) { if (glyphs_.erase(c)) changed(); }
void R2CharacterLcd::clearCharacterSet() { glyphs_.clear(); changed(); }
void R2CharacterLcd::clear(CharacterCode c)
{
    if (std::all_of(cells_.begin(), cells_.end(), [c](auto v) { return v == c; })) return;
    std::fill(cells_.begin(), cells_.end(), c);
    changed();
}
bool R2CharacterLcd::setCell(int column, int row, CharacterCode c)
{
    if (column < 0 || column >= layout_.columns || row < 0 || row >= layout_.rows) return false;
    auto& cell = cells_[static_cast<std::size_t>(row * layout_.columns + column)];
    if (cell != c) { cell = c; changed(); }
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
    blinkOn_ = true; blinkPhaseMs_ = 0;
    updateTimer();
    changed();
}
void R2CharacterLcd::setDisplayEnabled(bool e)
{
    if (enabled_ == e) return;
    enabled_ = e;
    updateTimer();
    changed();
}
// A parent can show/hide this LCD without changing its own visible flag.
void R2CharacterLcd::componentPeerChanged() { updateTimer(); }
void R2CharacterLcd::componentVisibilityChanged() { updateTimer(); }
// One timer for both: the cursor's blink (its half period), and ~60 Hz while dots fade.
void R2CharacterLcd::updateTimer()
{
    const bool blink = enabled_ && isShowing() && cursor_.blinkOverlay != Cursor::Style::hidden;
    if (fading_ && isShowing()) { if (getTimerInterval() != 16) { lastTickMs_ = juce::Time::getMillisecondCounterHiRes(); startTimer(16); } }
    else if (blink) startTimer(cursor_.blinkPeriodMs / 2);
    else stopTimer();
}
void R2CharacterLcd::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes();
    const double dt = getTimerInterval() == 16 ? now - lastTickMs_ : cursor_.blinkPeriodMs / 2.0;
    lastTickMs_ = now;
    if (cursor_.blinkOverlay != Cursor::Style::hidden) {
        blinkPhaseMs_ += dt;
        if (blinkPhaseMs_ >= cursor_.blinkPeriodMs / 2.0) { blinkPhaseMs_ = 0; blinkOn_ = !blinkOn_; changed(); }
    }
    if (fading_) { advanceFade(dt); repaint(); }
    updateTimer();
}

void R2CharacterLcd::setFadeTime(int ms)
{
    fadeMs_ = std::max(0, ms);
    level_.clear(); fading_ = false;
    updateTimer();
    repaint();
}

void R2CharacterLcd::changed()
{
    repaint();
    if (fadeMs_ <= 0) return;
    fading_ = true;                       // (advanceFade finds out whether anything moves)
    updateTimer();
}

bool R2CharacterLcd::dotLit(int column, int row, int x, int y) const
{
    const auto glyph = glyphs_.find(cells_[static_cast<std::size_t>(row * layout_.columns + column)]);
    const bool cursorHere = cursor_.column == column && cursor_.row == row;
    const auto cursorDot = [this, y](Cursor::Style style) {
        return style == Cursor::Style::block
            || (style == Cursor::Style::underline && y == layout_.glyphHeight - 1);
    };
    return enabled_ && ((glyph != glyphs_.end()
        && glyph->second.dots[static_cast<std::size_t>(y * layout_.glyphWidth + x)] != 0)
        || (cursorHere && (cursorDot(cursor_.style) || (blinkOn_ && cursorDot(cursor_.blinkOverlay)))));
}

bool R2CharacterLcd::advanceFade(double elapsedMs)
{
    const int w = layout_.columns * layout_.glyphWidth, n = w * layout_.rows * layout_.glyphHeight;
    const bool first = level_.size() != static_cast<std::size_t>(n);
    if (first) level_.assign(static_cast<std::size_t>(n), 0.0f);
    const float step = fadeMs_ > 0 ? static_cast<float>(elapsedMs / fadeMs_) : 1.0f;
    bool moving = false;
    for (int row = 0; row < layout_.rows; ++row)
        for (int column = 0; column < layout_.columns; ++column)
            for (int y = 0; y < layout_.glyphHeight; ++y)
                for (int x = 0; x < layout_.glyphWidth; ++x)
                {
                    auto& l = level_[static_cast<std::size_t>((row * layout_.glyphHeight + y) * w + column * layout_.glyphWidth + x)];
                    const float target = dotLit(column, row, x, y) ? 1.0f : 0.0f;
                    if (first) { l = target; continue; }      // (the first frame as it is)
                    l = l < target ? std::min(target, l + step) : std::max(target, l - step);
                    moving = moving || l != target;
                }
    fading_ = moving;
    return moving;
}

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
    // the dots by brightness: unlit, lit, and (fading) 15 steps between
    constexpr int kSteps = 16;
    juce::RectangleList<float> shade[kSteps + 1];
    const int w = layout_.columns * layout_.glyphWidth;
    const bool levels = fadeMs_ > 0 && level_.size() == static_cast<std::size_t>(w * layout_.rows * layout_.glyphHeight);
    for (int row = 0; row < layout_.rows; ++row)
        for (int column = 0; column < layout_.columns; ++column)
            for (int y = 0; y < layout_.glyphHeight; ++y)
                for (int x = 0; x < layout_.glyphWidth; ++x)
                {
                    int k = dotLit(column, row, x, y) ? kSteps : 0;
                    if (levels)
                        k = juce::roundToInt(level_[static_cast<std::size_t>((row * layout_.glyphHeight + y) * w
                                                                           + column * layout_.glyphWidth + x)] * kSteps);
                    shade[k].addWithoutMerging({x0 + column * (cw + cg) + x * (dw + gx),
                        y0 + row * (ch + lg) + y * (dh + gy), dw, dh});
                }
    for (int k = 0; k <= kSteps; ++k)
    {
        if (shade[k].isEmpty()) continue;
        g.setColour(palette_.unlit.interpolatedWith(palette_.lit, static_cast<float>(k) / kSteps));
        g.fillRectList(shade[k]);
    }
}
}
