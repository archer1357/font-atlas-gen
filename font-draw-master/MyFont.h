
#include <emscripten/bind.h>
#include <emscripten/val.h>


using namespace emscripten;

#include <stdio.h>
#include <string>

#include "font.h"

class MyFont {
public:
    std::vector<uint8_t> buffer;
    stbtt_fontinfo fontInfo;
    int rowAdvance;
    float scale;
    int scaledAscent;
    std::vector<FontGlyph> glyphs;

    struct SimpleFont simpleFont;
    std::vector<uint8_t> bits;
    int texWidth,texHeight;
public:
    MyFont();

    bool initFontInfo(const emscripten::val &fontInfoData);
    void calcAttribs(int pointSize);
    void initAsciiGlyphs();
    void packGlyphs();
    void bitmapGlyphs();

};
