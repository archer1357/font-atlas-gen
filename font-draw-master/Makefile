OUTPUT=../MyFont.js
CC=/usr/lib/emscripten/em++
CXXFLAGS := -x c++ --std=c++11 -s WASM=0

sources:= MyFont.cpp font.c BinPacker.cpp
headers:= MyFont.h font.h stb_truetype.h BinPacker.hpp

all: $(sources) $(OUTPUT)

.PHONY: all test

$(OUTPUT): $(sources) $(headers)
	@mkdir -p $(@D)
	$(CC) $(CXXFLAGS) $(sources) --bind -o $(OUTPUT) 
