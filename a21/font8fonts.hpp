//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include "font8.hpp"

namespace a21 {
	
/**
 * 8-bit font data generated from 'PixelstadTweaked'.
 */
class Font8PixelstadTweaked {
public:
   	static Font8::Data data() {
   		static const uint8_t PROGMEM _data[] = {
   			// Flags: bit 0 - uppercase only
   			1,

   			// Range ' ' to '`'.
   			// From/to/bytes per character.
   			32, 96, 6,

   			// For each character in the range:
   			// Actual width of the character, M;
   			// M bytes with the pixel data of the character, one byte per column of pixels;
   			// K zeros so M + K + 1 = bytes per characters for the range.
   			/* ' ' */ 3, 0, 0, 0, 0, 0,
   			/* '!' */ 1, 94, 0, 0, 0, 0,
   			/* '"' */ 3, 6, 0, 6, 0, 0,
   			/* '#' */ 5, 40, 124, 40, 124, 40,
   			/* '$' */ 3, 92, 214, 116, 0, 0,
   			/* '%' */ 3, 100, 16, 76, 0, 0,
   			/* '&' */ 4, 52, 74, 52, 80, 0,
   			/* ''' */ 1, 6, 0, 0, 0, 0,
   			/* '(' */ 2, 124, 130, 0, 0, 0,
   			/* ')' */ 2, 130, 124, 0, 0, 0,
   			/* '*' */ 3, 20, 8, 20, 0, 0,
   			/* '+' */ 5, 16, 16, 124, 16, 16,
   			/* ',' */ 1, 192, 0, 0, 0, 0,
   			/* '-' */ 4, 16, 16, 16, 16, 0,
   			/* '.' */ 1, 64, 0, 0, 0, 0,
   			/* '/' */ 3, 96, 16, 12, 0, 0,
   			/* '0' */ 3, 124, 68, 124, 0, 0,
   			/* '1' */ 3, 72, 124, 64, 0, 0,
   			/* '2' */ 3, 116, 84, 92, 0, 0,
   			/* '3' */ 3, 68, 84, 124, 0, 0,
   			/* '4' */ 3, 28, 16, 124, 0, 0,
   			/* '5' */ 3, 92, 84, 116, 0, 0,
   			/* '6' */ 3, 124, 84, 116, 0, 0,
   			/* '7' */ 3, 4, 116, 12, 0, 0,
   			/* '8' */ 3, 124, 84, 124, 0, 0,
   			/* '9' */ 3, 92, 84, 124, 0, 0,
   			/* ':' */ 1, 72, 0, 0, 0, 0,
   			/* ';' */ 1, 200, 0, 0, 0, 0,
   			/* '<' */ 3, 16, 40, 68, 0, 0,
   			/* '=' */ 4, 40, 40, 40, 40, 0,
   			/* '>' */ 3, 68, 40, 16, 0, 0,
   			/* '?' */ 3, 4, 82, 12, 0, 0,
   			/* '@' */ 4, 120, 132, 180, 56, 0,
   			/* 'A' */ 3, 120, 20, 124, 0, 0,
   			/* 'B' */ 3, 124, 84, 40, 0, 0,
   			/* 'C' */ 3, 56, 68, 68, 0, 0,
   			/* 'D' */ 3, 124, 68, 56, 0, 0,
   			/* 'E' */ 3, 124, 84, 68, 0, 0,
   			/* 'F' */ 3, 124, 20, 4, 0, 0,
   			/* 'G' */ 3, 124, 68, 116, 0, 0,
   			/* 'H' */ 3, 124, 16, 124, 0, 0,
   			/* 'I' */ 3, 68, 124, 68, 0, 0,
   			/* 'J' */ 3, 32, 64, 60, 0, 0,
   			/* 'K' */ 3, 124, 16, 108, 0, 0,
   			/* 'L' */ 3, 124, 64, 64, 0, 0,
   			/* 'M' */ 5, 124, 4, 124, 4, 120,
   			/* 'N' */ 3, 124, 4, 120, 0, 0,
   			/* 'O' */ 3, 124, 68, 124, 0, 0,
   			/* 'P' */ 3, 124, 20, 28, 0, 0,
   			/* 'Q' */ 4, 124, 68, 124, 64, 0,
   			/* 'R' */ 3, 124, 20, 104, 0, 0,
   			/* 'S' */ 3, 92, 84, 116, 0, 0,
   			/* 'T' */ 3, 4, 124, 4, 0, 0,
   			/* 'U' */ 3, 124, 64, 124, 0, 0,
   			/* 'V' */ 3, 60, 64, 60, 0, 0,
   			/* 'W' */ 5, 60, 64, 48, 64, 60,
   			/* 'X' */ 3, 108, 16, 108, 0, 0,
   			/* 'Y' */ 3, 92, 80, 124, 0, 0,
   			/* 'Z' */ 3, 100, 84, 76, 0, 0,
   			/* '[' */ 2, 254, 130, 0, 0, 0,
   			/* '\' */ 3, 12, 16, 96, 0, 0,
   			/* ']' */ 2, 130, 254, 0, 0, 0,
   			/* '^' */ 3, 4, 2, 4, 0, 0,
   			/* '_' */ 4, 128, 128, 128, 128, 0,
   			/* '`' */ 2, 2, 4, 0, 0, 0,

   			// Range '{' to '~'.
   			123, 126, 5,
   			/* '{' */ 3, 16, 254, 130, 0,
   			/* '|' */ 1, 254, 0, 0, 0,
   			/* '}' */ 3, 130, 254, 16, 0,
   			/* '~' */ 4, 8, 4, 8, 4,

   			// End of all the ranges.
   			0
   		};
   		return _data;
   	}
};  

typedef Font8PixelstadTweaked Font8Console;

}; // namespace
