#define STB_TRUETYPE_IMPLEMENTATION
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void font_createBitmap(const stbtt_fontinfo *fontInfo,
                         const struct FontGlyph *glyphs,
                         int glyphsNum,float scale,int texWidth,
                         int texHeight,unsigned char *pBits) {
  //Originally taken from the Urho3D Engine
  int i,y,x;
  int Pitch=texWidth;
  memset(pBits,0,texWidth*texHeight);

  // Render glyphs into texture, and find out a scaling value in case font uses less than full opacity (thin outlines)
  int sumOpacity = 0;
  int nonEmptyGlyphs = 0;

  for(i = 0; i < glyphsNum; ++i)  {
    unsigned char *output;
    output=(unsigned char*)pBits + Pitch * glyphs[i].y + glyphs[i].x;
    stbtt_MakeGlyphBitmap(fontInfo,output,glyphs[i].width,glyphs[i].height,
                          Pitch,scale,scale,glyphs[i].index);

    int glyphMaxOpacity = 0;

    for( y = 0; y < glyphs[i].height; ++y) {
      unsigned char* pixels;
      pixels=(unsigned char*)pBits+Pitch*(y+glyphs[i].y)+glyphs[i].x;

      for(x = 0; x < glyphs[i].width; ++x) {
        if((int)pixels[x] > glyphMaxOpacity) {
          glyphMaxOpacity=(int)pixels[x];
        }
      }
    }

    if(glyphMaxOpacity > 0) {
      sumOpacity += glyphMaxOpacity;
      ++nonEmptyGlyphs;
    }
  }

  // Apply the scaling if necessary
  int avgOpacity = nonEmptyGlyphs ? sumOpacity / nonEmptyGlyphs : 255;

  if(avgOpacity < 255) {
    float scale = 255.0f / avgOpacity;

    for(i = 0; i < glyphsNum; ++i)  {
      for(y = 0; y < glyphs[i].height; ++y) {
        unsigned char* dest;
        dest=(unsigned char*)pBits+Pitch*(y+glyphs[i].y)+glyphs[i].x;

        for(x = 0; x < glyphs[i].width; ++x) {
          int aaa=(int)(dest[x]*scale);
          dest[x]=(aaa < 255)?aaa:255;
        }
      }
    }
  }
}

int font_glyphCompare(const void *a,const void *b) {
  const struct FontGlyph *aa,*bb;
  aa=*((const struct FontGlyph **)a);
  bb=*((const struct FontGlyph **)b);

  if(aa->height<bb->height) {
    return 1;
  }

  if(aa->height==bb->height) {
    return 0;
  }

  //if(aa->mHeight>bb->mHeight) {
    return -1;
  //}
}


void font_packGlyphs(struct FontGlyph *glyphs,
                       int glyphsNum,int padding,
                       int *pTexWidth,int *pTexHeight) {

  //todo: when glyph.height>glyph.width then flip them sideways
  // add sideways : bool to glyph struct
  // update glyphQuad func

  int i=0;
  int x=padding;
  int y=padding;
  int width=32;
  int height=32;
  int xEnd=0; //furthest x+glyphWidth has been entirely
  int xStart=padding; //where to start from after moving y down
  int yEnd=padding; //furthest y+glyphHeight has been entirely
  int yStep=0; //furthest y+glyphHeight has been within current area
  struct FontGlyph **sortedGlyphs;
  int j;

  sortedGlyphs=(struct FontGlyph **)malloc(sizeof(void*)*glyphsNum);

  for(j=0;j<glyphsNum;j++) {
    sortedGlyphs[j]=&glyphs[j];
  }

  qsort(sortedGlyphs, glyphsNum, sizeof(void*), font_glyphCompare);

  while(i<glyphsNum) {
    struct FontGlyph *glyph=sortedGlyphs[i];

    if(x+glyph->width+padding <= width &&
       y+glyph->height+padding <= height) { //go across
      glyph->x=x;
      glyph->y=y;
      x+=glyph->width+padding;

      //set furthest down within area
      if(y+glyph->height+padding > yStep) {
        yStep=y+glyph->height+padding;
      }

      //set furthest across
      if(x > xEnd) {
        xEnd=x;
      }

      //set furthest down
      if(yStep>yEnd) {
        yEnd=yStep;
      }

      i++;
    } else if(x+glyph->width+padding > width &&
              yStep+glyph->height+padding<=height) { //can't go across, go down
      //set x to last starting point, like a typewriter
      x=xStart;

      //set y down to furthest down within area
      y=yStep;
    } else if(width<=height) { //can't go across, can't go down, expand rightward
      //set x to furthest across
      xStart=x=xEnd;

      //y and yStep goes to zero since we are expanding rightward
      y=yStep=padding;

      //expand across
      width*=2;
    } else { //can't go across, can't go down, expand downward
      //x and xStart goes to zero since we are expanding downward
      x=xStart=padding;

      //set y to furthest down
      y=yEnd;

      //expand downward
      height*=2;
    }
  }

  //
  free(sortedGlyphs);

  //
  *pTexWidth=width;
  *pTexHeight=height;


}

void font_glyphQuad(const struct FontGlyph *glyph,
                      bool flip,float rowAdvance,
                      float texInvWidth,float texInvHeight,
                      float x, float y,
                      stbtt_aligned_quad *q) {

  if(flip) {
    q->y0 = y+(float)glyph->offsetY; //use floorf?
    q->y1 = q->y0 + (float)glyph->height;
  } else {
    q->y0 = y+(float)(rowAdvance-glyph->offsetY); //use floorf?
    q->y1 = q->y0 - (float)glyph->height;
  }

  q->x0 = x + (float)glyph->offsetX; //use floorf?
  q->x1 = q->x0 + (float)glyph->width;
  q->s0 = glyph->x*texInvWidth;
  q->t0 = (glyph->y + glyph->height)*texInvHeight;
  q->s1 = (glyph->x + glyph->width)*texInvWidth;
  q->t1 = glyph->y*texInvHeight;


}

void font_initGlyph(const stbtt_fontinfo *fontInfo,float scale,int scaledAscent,int ii,struct FontGlyph *glyph) {

    int mGlyphIndex = stbtt_FindGlyphIndex(fontInfo,ii);

    int ix0,iy0,ix1,iy1;
    int advanceWidth,leftSideBearing;

    stbtt_GetGlyphBitmapBox(fontInfo,mGlyphIndex,scale,scale,&ix0,&iy0,&ix1,&iy1);
    stbtt_GetGlyphHMetrics(fontInfo,mGlyphIndex,&advanceWidth,&leftSideBearing);
    glyph->width = ix1 - ix0;
    glyph->height = iy1 - iy0;
    glyph->offsetX = (int)(leftSideBearing * scale);
    glyph->offsetY = iy0+scaledAscent; //top align
    glyph->colAdvance = (int)(advanceWidth * scale);
    glyph->index=mGlyphIndex;
}

void font_asciiTextGlyphs(const stbtt_fontinfo *fontInfo,
                            float scale,int scaledAscent,
                            struct FontGlyph *glyphs) {
  //Originally taken from the Urho3D Engine
  // Go through glyphs to get their dimensions & offsets
                                
  int i;

  for(i = 0; i < 95; ++i)  {
      font_initGlyph(fontInfo,scale,scaledAscent,i+32,&glyphs[i]);
  }
  
      font_initGlyph(fontInfo,scale,scaledAscent,0,&glyphs[95]);
}

void font_calcAttribs(const stbtt_fontinfo *fontInfo,int pointSize,float *pScale, int *pScaledAscent, int *pRowAdvance) {
 int ascent, descent,lineGap;  

  //Get row height
  stbtt_GetFontVMetrics(fontInfo,&ascent,&descent,&lineGap);

  //Calculate scale (use ascent only)
  *pScale=(float)pointSize/(float)ascent;
    
    //
    float scale =*pScale;
                            
    //                    
    *pScaledAscent = (int)(scale* (float)ascent);

  //Calculate row advance
  *pRowAdvance=(int)(scale*(float)(ascent-descent+lineGap));
}

void font_simpleCreate(const stbtt_fontinfo *fontInfo,
                        int pointSize,
                        struct SimpleFont *simpleFont,
                        unsigned char **pBits) {

  int scaledAscent;
  float scale;
  int w,h;
                            
    font_calcAttribs(fontInfo,pointSize,&scale,&scaledAscent, &simpleFont->rowAdvance) ;


  //
  font_asciiTextGlyphs(fontInfo,scale,scaledAscent,simpleFont->glyphs);
  font_packGlyphs(simpleFont->glyphs,FONT_ASCII_TEXT_GLYPHS,1,&w,&h);

  //create bitmap
  *pBits=(unsigned char*)malloc(w*h);
  font_createBitmap(fontInfo,simpleFont->glyphs,FONT_ASCII_TEXT_GLYPHS,scale,w,h,*pBits);

  //
  simpleFont->texWidth=w;
  simpleFont->texHeight=h;
}

void font_simpleCharGlyph(const struct SimpleFont *simpleFont,
                          char c,struct FontGlyph *glyph) {
  const int glyphsFrom=32;
  const int glyphsTo=126;

  if(c >= glyphsFrom && c <= glyphsTo) {
    *glyph=simpleFont->glyphs[c-glyphsFrom];
  } else { //else use null char
    *glyph=simpleFont->glyphs[95];
  }

}

void font_simpleDrawVerts(const char *str,int strCount,float x,float y,
                           const struct SimpleFont *simpleFont,
                           bool flip,bool ccw,
                           float *verts, int *vertsNum) {

  float texInvWidth=1.0f/(float)simpleFont->texWidth;
  float texInvHeight=1.0f/(float)simpleFont->texHeight;
  float x2=x;
  const char *strEnd=&str[strCount];
  *vertsNum=0;

  while(str[0] && str < strEnd) {
    if(str[0]=='\n') {
      y+=(float)simpleFont->rowAdvance;
      x2=x;
    } else {
      struct FontGlyph glyph;
      font_simpleCharGlyph(simpleFont,str[0],&glyph);

      //
      stbtt_aligned_quad q;
      font_glyphQuad(&glyph,flip,(float)simpleFont->rowAdvance,
                       texInvWidth,texInvHeight,x2,y,&q);
      font_quadVerts(&q,verts,ccw);


      verts+=24;
      x2+=(float)glyph.colAdvance;
      (*vertsNum)+=6;
    }

    ++str;
  }
}
