#include "../modules/R2JUCE/src/R2CharacterLcd.h"
#include "../modules/R2JUCE/src/R2Hd44780A00Font.h"
#include <cstdlib>
#include <iostream>
static void require(bool ok, const char* message)
{
    if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    r2juce::R2CharacterLcd lcd;
    r2juce::R2CharacterLcd::Layout layout;
    layout.columns = 2; layout.rows = 2;
    layout.glyphWidth = 2; layout.glyphHeight = 2;
    layout.dotWidth = 3; layout.dotHeight = 3;
    layout.dotGapX = 1; layout.dotGapY = 1;
    layout.characterGap = 4; layout.lineGap = 6;
    layout.padding = juce::BorderSize<float>(2);
    require(lcd.setLayout(layout), "layout rejected");
    lcd.setPalette({juce::Colours::green, juce::Colours::black, juce::Colours::black});
    require(lcd.registerCharacter(0, {{1,1,1,1}}), "code zero registration failed");
    lcd.clear(0);
    lcd.setSize(22,24);
    auto render = [&] {
        juce::Image image(juce::Image::ARGB, lcd.getWidth(), lcd.getHeight(), true);
        juce::Graphics graphics(image);
        lcd.paintEntireComponent(graphics, true);
        return image;
    };
    // Device-scale rendering can be checked without creating a native window.
    for (int deviceScale : {2,3,4})
    {
        juce::Image scaled(juce::Image::ARGB,22*deviceScale,24*deviceScale,true);
        juce::Graphics graphics(scaled);
        graphics.addTransform(juce::AffineTransform::scale(static_cast<float>(deviceScale)));
        lcd.paintEntireComponent(graphics,true);
        for (int y=0;y<scaled.getHeight();++y)
            for (int x=0;x<scaled.getWidth();++x)
            {
                const int logicalX=x/deviceScale, logicalY=y/deviceScale;
                const bool dx=(logicalX>=2 && logicalX<5) || (logicalX>=6 && logicalX<9)
                    || (logicalX>=13 && logicalX<16) || (logicalX>=17 && logicalX<20);
                const bool dy=(logicalY>=2 && logicalY<5) || (logicalY>=6 && logicalY<9)
                    || (logicalY>=15 && logicalY<18) || (logicalY>=19 && logicalY<22);
                require(scaled.getPixelAt(x,y)==(dx && dy ? juce::Colours::green : juce::Colours::black),
                    "device scaling must preserve dot, character and line gaps");
            }
    }
    lcd.setScaling(r2juce::R2CharacterLcd::Scaling::fit);
    for (float deviceScale : {1.25f,1.5f,1.75f,2.5f})
    {
        juce::Image scaled(juce::Image::ARGB,static_cast<int>(std::ceil(22*deviceScale)),
            static_cast<int>(std::ceil(24*deviceScale)),true);
        juce::Graphics graphics(scaled);
        graphics.addTransform(juce::AffineTransform::scale(deviceScale));
        lcd.paintEntireComponent(graphics,true);
        const auto pixel=[&](float x,float y) {
            return scaled.getPixelAt(static_cast<int>(x*deviceScale),static_cast<int>(y*deviceScale));
        };
        require(pixel(3.5f,3.5f)==juce::Colours::green,
            "fractional fit must retain the interior of a lit dot");
        require(pixel(5.5f,3.5f).getGreen()<juce::Colours::green.getGreen()/2
            && pixel(3.5f,5.5f).getGreen()<juce::Colours::green.getGreen()/2,
            "fractional fit must retain visible gaps between dots");
        require(pixel(11,3.5f)==juce::Colours::black && pixel(3.5f,12)==juce::Colours::black,
            "fractional fit must retain character and row separation");
    }
    lcd.setScaling(r2juce::R2CharacterLcd::Scaling::pixelAligned);
    auto image = render();
    for (int y=0;y<24;++y)
        for (int x=0;x<22;++x)
        {
            const bool dx = (x>=2 && x<5) || (x>=6 && x<9) || (x>=13 && x<16) || (x>=17 && x<20);
            const bool dy = (y>=2 && y<5) || (y>=6 && y<9) || (y>=15 && y<18) || (y>=19 && y<22);
            require(image.getPixelAt(x,y)==(dx && dy ? juce::Colours::green : juce::Colours::black),
                "dot, character or line gap mismatch");
        }
    require(lcd.registerCharacter(0, {{1,0,0,0}}), "glyph replacement failed");
    image=render();
    require(image.getPixelAt(2,2)==juce::Colours::green && image.getPixelAt(6,2)==juce::Colours::black,
        "visible custom glyph did not update");
    // Render cursor forms without a native window or message-loop timing.
    lcd.setCell(1,1,U' ');
    for (const auto style : {r2juce::R2CharacterLcd::Cursor::Style::underline, r2juce::R2CharacterLcd::Cursor::Style::block})
    {
        lcd.setCursor({1,1,style});
        image = render();
        for (int y = 15; y < 22; ++y)
            for (int x = 13; x < 20; ++x)
            {
                const bool dotX = x < 16 || x >= 17;
                const bool dotY = (style == r2juce::R2CharacterLcd::Cursor::Style::block && y < 18) || y >= 19;
                require(image.getPixelAt(x,y) == (dotX && dotY ? juce::Colours::green : juce::Colours::black),
                    "cursor shape must preserve dot gaps and underline row");
            }
        require(image.getPixelAt(6,2) == juce::Colours::black, "cursor leaked into another cell");
    }
    lcd.setCursor({1,1,r2juce::R2CharacterLcd::Cursor::Style::hidden,r2juce::R2CharacterLcd::Cursor::Style::block});
    require(render().getPixelAt(13,15) == juce::Colours::green, "blink overlay initial phase must show its block");
    lcd.setDisplayEnabled(false);
    require(render().getPixelAt(13,15) == juce::Colours::black, "display off must suppress blinking cursor");
    lcd.setDisplayEnabled(true);
    lcd.setCursor({});
    require(render().getPixelAt(13,15) == juce::Colours::black, "hidden cursor must restore blank cell");
    require(!lcd.registerCharacter(1, {{1}}), "invalid glyph accepted");
    auto invalid=layout; invalid.columns=0;
    require(!lcd.setLayout(invalid), "invalid layout accepted");
    require(lcd.getLayout().columns==2, "invalid layout mutated display");
    lcd.setDisplayEnabled(false);
    require(render().getPixelAt(2,2)==juce::Colours::black, "display disable failed");
    lcd.setDisplayEnabled(true);
    require(render().getPixelAt(2,2)==juce::Colours::green, "display state lost on disable");
    require(!lcd.loadStandardCharacterSet(), "standard font accepted incompatible cell dimensions");
    {
        r2juce::R2CharacterLcd standard;
        r2juce::R2CharacterLcd::Layout l;
        l.columns=2; l.rows=1; l.dotGapX=0; l.dotGapY=0;
        l.characterGap=0; l.padding=juce::BorderSize<float>(0);
        require(standard.setLayout(l), "standard font test layout");
        standard.setSize(10,8);
        standard.setPalette({juce::Colours::white,juce::Colours::black,juce::Colours::black});
        require(standard.registerCharacter(0,{std::vector<std::uint8_t>(40,1)}), "custom font setup");
        require(standard.loadStandardCharacterSet(), "standard font load");
        standard.write(0,0,U"A"); standard.setCell(1,0,0);
        juce::Image pixels(juce::Image::RGB,10,8,true);
        juce::Graphics graphics(pixels); standard.paint(graphics);
        for (int y=0;y<8;++y)
            for (int x=0;x<5;++x)
            {
                const bool lit=(r2juce::detail::hd44780A00Rows['A'][y]>>(4-x))&1U;
                require(pixels.getPixelAt(x,y)==(lit?juce::Colours::white:juce::Colours::black),
                    "standard character bitmap mismatch");
                require(pixels.getPixelAt(5+x,y)==juce::Colours::white,
                    "loading standard font replaced a custom character");
            }
    }
    {
        r2juce::R2CharacterLcd reusable;
        r2juce::R2CharacterLcd::Layout l;
        l.columns = 20; l.rows = 4; l.glyphWidth = l.glyphHeight = 1;
        l.dotWidth = l.dotHeight = 1; l.dotGapX = l.dotGapY = 0;
        l.characterGap = 1; l.lineGap = 2; l.padding = juce::BorderSize<float>(0);
        require(reusable.setLayout(l), "20x4 reusable layout rejected");
        reusable.setPalette({juce::Colours::red, juce::Colours::blue, juce::Colours::yellow});
        require(reusable.registerCharacter(U'A', {{1}})
            && reusable.registerCharacter(U' ', {{0}}), "20x4 custom glyphs rejected");
        reusable.write(0, 0, U"AAAAAAAAAAAAAAAAAAAAA"); // Clip the 21st character.
        reusable.write(19, 3, U"AA");
        reusable.setSize(39, 10);
        juce::Image pixels(juce::Image::ARGB, 39, 10, true);
        juce::Graphics graphics(pixels);
        reusable.paint(graphics);
        for (int y = 0; y < 10; ++y)
            for (int x = 0; x < 39; ++x)
            {
                const bool dot = y % 3 == 0 && x % 2 == 0;
                const bool lit = dot && (y == 0 || (y == 9 && x == 38));
                const auto expected = lit ? juce::Colours::red
                    : dot ? juce::Colours::blue : juce::Colours::yellow;
                require(pixels.getPixelAt(x,y) == expected,
                    "20x4 text clipping, palette or inter-cell spacing mismatch");
            }
    }
    r2juce::R2CharacterLcd rowsLcd;
    r2juce::R2CharacterLcd::Layout rowsLayout;
    rowsLayout.columns = rowsLayout.rows = 1;
    rowsLayout.dotGapX = rowsLayout.dotGapY = 0;
    rowsLayout.padding = juce::BorderSize<float>(0);
    require(rowsLcd.setLayout(rowsLayout), "rows layout");
    rowsLcd.setSize(5,8);
    rowsLcd.setPalette({juce::Colours::white, juce::Colours::black, juce::Colours::black});
    const std::array<std::uint8_t,8> pattern { 0xf0,8,4,2,1,31,0,21 };
    require(rowsLcd.registerCharacterRows(0,pattern), "row registration");
    require(rowsLcd.loadStandardCharacterSet(), "standard font preserves row glyph");
    rowsLcd.setCell(0,0,0);
    juce::Image rowPixels(juce::Image::ARGB,5,8,true);
    juce::Graphics rowGraphics(rowPixels);
    rowsLcd.paint(rowGraphics);
    for (int y=0;y<8;++y)
        for (int x=0;x<5;++x)
            require(rowPixels.getPixelAt(x,y)==(((pattern[y]>>(4-x))&1) ? juce::Colours::white : juce::Colours::black), "row bit order or masking");
    require(rowsLcd.registerCharacterRows(0,{}), "replace row glyph");
    rowsLcd.paint(rowGraphics);
    require(rowPixels.getPixelAt(0,0)==juce::Colours::black, "replacement must update existing cell");
    rowsLayout.glyphWidth=6;
    rowsLcd.setLayout(rowsLayout);
    require(!rowsLcd.registerCharacterRows(0,pattern), "reject incompatible layout");
    if (argc>1)
    {
        r2juce::R2CharacterLcd preview;
        r2juce::R2CharacterLcd::Layout l; l.rows=1;
        preview.setLayout(l);
        preview.loadStandardCharacterSet();
        preview.registerCharacterRows(0,pattern);
        preview.setScaling(r2juce::R2CharacterLcd::Scaling::fit);
        preview.setPalette({juce::Colours::black,juce::Colours::transparentBlack,juce::Colour(0xff86a070)});
        preview.write(0,0,U"MIDI CHANNEL  01");
        preview.setSize(576,96);
        juce::Image pixels(juce::Image::ARGB,576,96,true);
        juce::Graphics g(pixels); preview.paintEntireComponent(g,true);
        auto stream=juce::File(argv[1]).createOutputStream();
        require(stream!=nullptr,"preview output");
        stream->setPosition(0); stream->truncate();
        require(juce::PNGImageFormat().writeImageToStream(pixels,*stream),"preview write");
    }
    std::cout << "Character LCD rendering checks passed\n";
}
