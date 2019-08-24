
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"


#include "MyFont.h"


MyFont::MyFont() {

}



bool MyFont::initFontInfo(const emscripten::val &fontInfoData) {
    this->buffer.resize(fontInfoData["length"].as<unsigned int>());
    
    fontInfoData["constructor"]
        .new_(emscripten::val::module_property("buffer"), 
        reinterpret_cast<uintptr_t>(buffer.data()), 
        this->buffer.size())
        .call<void>("set", fontInfoData);
    
  if(!stbtt_InitFont(&this->fontInfo,this->buffer.data(),0)) {
    fprintf(stderr,"font load error\n");
    return false;
  }
    
    return true;
}

void MyFont::calcAttribs(int pointSize) {
    font_calcAttribs(&this->fontInfo,pointSize,&this->scale,&this->scaledAscent, &this->rowAdvance) ;
}

void MyFont::initAsciiGlyphs() {
    this->glyphs.resize(96);
    font_asciiTextGlyphs(&this->fontInfo,this->scale,this->scaledAscent,this->glyphs.data());
}

void MyFont::packGlyphs() {
    //font_packGlyphs(this->glyphs.data(),this->glyphs.size(),1,&this->texWidth,&this->texHeight);
    
    /*BinPacker bp
    std::vector< int > rectangle_info;
	std::vector< std::vector<int> > packed_glyph_info;
    int pack_tex_size;
    
    
    bp.Pack(rectangle_info, packed_glyph_info, pack_tex_size, false);
    ;*/
    
    stbrp_context context;

    std::vector<struct stbrp_rect> rects;
    rects.resize(this->glyphs.size());

    for (int i=0; i< rects.size(); i++)    {
        struct FontGlyph glyph=this->glyphs[i];

        rects[i].id = i;
        rects[i].w = glyph.width;
        rects[i].h = glyph.height;
        rects[i].x = glyph.x;
        rects[i].y = glyph.y;
        rects[i].was_packed = 0;
    }
   
    int texWidth=32;
    int texHeight=32;
    
    
        std::vector<struct stbrp_node> nodes;
    nodes.resize(texWidth*2);

    
    
    while(true) {
        
    stbrp_init_target(&context, texWidth, texHeight, nodes.data(), nodes.size());
    stbrp_pack_rects(&context, rects.data(), rects.size());
    
        
    bool done=true;
        
        for (int i=0; i< rects.size(); i++)   {
            if(!rects[i].was_packed) {
                done=false;
                break;
            }
        }
        
        if(done) {
            break;
        }
        
        if(texWidth>texHeight) {
            texHeight*=2;
        } else {
            texWidth*=2;
            nodes.resize(texWidth*2);
        }
            
    }


    for (int i=0; i< rects.size(); i++)    {
        int id=rects[i].id;
        struct FontGlyph *glyph=&this->glyphs[id];
        glyph->x=rects[i].x;
        glyph->y=rects[i].y;
        //glyph->width=rects[i].w;
        //glyph->height=rects[i].h;
         
    }
    
       
   
    
    this->texWidth=context.width;
    this->texHeight=context.height;

    for (int i=0; i< 96; i++)   {
        if(!rects[i].was_packed) {
            printf("not %i\n",i);
        }
        //printf("rect %i (%hu,%hu) was_packed=%i\n", rects[i].id, rects[i].x, rects[i].y, rects[i].was_packed);
    }
    
}

void MyFont::bitmapGlyphs() {
    this->bits.resize(this->texWidth*this->texHeight);
    font_createBitmap(&this->fontInfo,this->glyphs.data(),this->glyphs.size(),this->scale,this->texWidth,this->texHeight,this->bits.data());
}

EMSCRIPTEN_BINDINGS(my_module) {
    register_vector<uint8_t>("vector<uint8_t>");
    register_vector<FontGlyph>("vector<FontGlyph>");
    
    class_<MyFont>("MyFont")
        .constructor<>()
        .function("initFontInfo", &MyFont::initFontInfo)
        .function("calcAttribs", &MyFont::calcAttribs)
        .function("initAsciiGlyphs", &MyFont::initAsciiGlyphs)
        .function("packGlyphs", &MyFont::packGlyphs)
        .function("bitmapGlyphs", &MyFont::bitmapGlyphs)
        .property("rowAdvance", &MyFont::rowAdvance)
        .property("texWidth", &MyFont::texWidth)
        .property("texHeight", &MyFont::texHeight)
        .property("bits", &MyFont::bits)
        .property("glyphs", &MyFont::glyphs)
    ;

    value_object<FontGlyph>("FontGlyph")
        .field("x", &FontGlyph::x)
        .field("y", &FontGlyph::y)
        .field("width", &FontGlyph::width)
        .field("height", &FontGlyph::height)
        .field("offsetX", &FontGlyph::offsetX)
        .field("offsetY", &FontGlyph::offsetY)
        .field("colAdvance", &FontGlyph::colAdvance)
        .field("index", &FontGlyph::index)
        ;
}
    
/*

emscripten::val MyFont::getBits() {
    int bufferLength=this->simpleFont.texWidth*this->simpleFont.texHeight;
    return emscripten::val(emscripten::typed_memory_view(bufferLength, this->bits));
}


//~ emscripten::val MyFont::getSimpleFont() {
    //~ return this->simpleFont;
//~ }

FontGlyph MyFont::getGlyph(int i) {
    return simpleFont.glyphs[i];
}

EMSCRIPTEN_BINDINGS(my_module) {
    class_<MyFont>("MyFont")
        .constructor<>()
        .function("init", &MyFont::init)
        //.function("init", &MyFont::init,allow_raw_pointers())
        .function("uninit", &MyFont::uninit)
        .function("getBits", &MyFont::getBits)
        //.function("getSimpleFont", &MyFont::getSimpleFont)
    .property("simpleFont", &MyFont::simpleFont)
    
    .function("getGlyph", &MyFont::getGlyph)
    ;

    
    value_object<SimpleFont>("SimpleFont")
        //.field("glyphs", &SimpleFont::glyphs)
        .field("rowAdvance", &SimpleFont::rowAdvance)
        .field("texWidth", &SimpleFont::texWidth)
        .field("texHeight", &SimpleFont::texHeight)
        ;

}
*/