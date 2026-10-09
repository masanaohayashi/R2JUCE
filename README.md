# R2JUCE

Reusable JUCE components in the `R2JUCE` module.

## R2CharacterLcd

A dot-matrix character LCD adapted from TX81Z's CharacterLcdComponent. The
component owns display data only; call its methods on the message thread.

```cpp
r2juce::R2CharacterLcd lcd;
r2juce::R2CharacterLcd::Layout layout;
layout.columns = 16;
layout.rows = 1;
lcd.setLayout(layout);
lcd.loadStandardCharacterSet(); // HD44780 A00, 5x8 cells
lcd.setScaling(r2juce::R2CharacterLcd::Scaling::fit);
lcd.setBounds(0, 0, 288, 48);
addAndMakeVisible(lcd);
lcd.write(0, 0, U"HELLO");

// Eight rows, low five bits per row; bit 4 is the leftmost dot.
lcd.registerCharacterRows(0, {4, 14, 21, 4, 4, 4, 0, 0});
lcd.setCell(6, 0, 0);
```

`registerCharacter(code, Glyph)` accepts a row-major dot array for arbitrary
glyph sizes. Registering the same code replaces its glyph and updates existing
cells. `removeCharacter()` and `clearCharacterSet()` remove definitions;
`clear()` clears the text while preserving definitions. The standard character
set uses codes 16–255 and preserves custom definitions at codes 0–15.

`Layout` controls rows, columns, glyph size, dot size, spacing, and padding.
`setLayout()` clears text and resets the cursor; changing glyph dimensions also
clears glyph definitions. Invalid layouts or glyph sizes return `false`.
`write()` clips at the end of the current row. Unknown glyphs appear blank.

`setPalette()` sets lit dots, unlit dots, and background, including transparent
colours. `Scaling::fit` fits the full display proportionally; `pixelAligned`
(default) aligns dots to physical pixels and can clip when bounds are too small.
`setCursor()` supports underline, block, and blinking overlays.
`setDisplayEnabled(false)` hides lit dots while retaining display data.

### Rendering checks

```sh
cmake -S tests -B /tmp/r2-lcd-tests -DJUCE_SOURCE_DIR=/path/to/JUCE -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/r2-lcd-tests --parallel 4
ctest --test-dir /tmp/r2-lcd-tests --output-on-failure
```
