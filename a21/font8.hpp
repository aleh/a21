//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

namespace a21 {

/**
 * Support for simple 8 pixel-high fonts (the ones exactly fitting 8 bit rows of popular monochrome displays 
 * we support here: SSD1306-compatible OLEDs and PCD8544-compatible LCDs, aka "Nokia LCDs").
 */
class Font8 {
	 
public:

	// Typedef for a font data binary stored in the flash.
	// 
	// The first byte contains flags: 
	// - currently only bit 0 is used; when set, then the font contains no lowercase English characters.
	//
	// Next follow one or more character ranges, each with a 3 byte header:
	// - first character in the range, f; when this is 0, then there are no character ranges following anymore;
	// - last character in the range, l;
	// - number of bytes per character in this range, N.
	//
	// Then follow (f - l + 1) groups of N bytes each describing a character of the range, from f to l. 
	//
	// In every group:
	// - the first byte contains the actual width of the character, W, i.e. how many pixel columns the character 
	//   should occupy when rendered on the screen, W <= N - 1.
	// - the next (N - 1) bytes contain the actual 8-pixel-high bitmap (where only the first W bytes are used).
	typedef const uint8_t *Data;
    
	/** 
	 * Returns the width of the glyph corresponding to a character in the given font; if a buffer is provided, 
	 * then copies glyph's bitmap bytes into it. 
	 */
	static uint8_t dataForCharacter(Data font, char ch, uint8_t *buffer) {

		const uint8_t *p = font;

		uint8_t options = pgm_read_byte(p++);
		if ((options & 1) && 'a' <= ch && ch <= 'z') {
			ch = ch - 'a' + 'A';
		}

		while (true) {

			// The first character in the range (0 would mean no more ranges are defined).
			uint8_t first = pgm_read_byte(p++);
			if (first == 0) 
				break;

			// The last character in the range.
			uint8_t last = pgm_read_byte(p++);

			// Number of bytes every character in the range occupies. 
			uint8_t bytes_per_character = pgm_read_byte(p++);

			// If our character is in the range, then copy its bitmap and return.
			if (first <= ch && ch <= last) {
    
				p += (ch - first) * bytes_per_character;

				// The first byte of the glyph data is the actual width of the glyph.
				uint8_t width = pgm_read_byte(p++);

				// Copy the bitmap if the caller expects it.
				if (buffer) {
					memcpy_PF(buffer, (uint_farptr_t)p, width);
				}

			  return width;
			}

			// Otherwise let's jump to the next range of characters.
			p += (last + 1 - first) * bytes_per_character;      
		}

		// Not found, let's return data for sort of a default character.
		// TODO: something we can make a parameter
		return dataForCharacter(font, '?', buffer);
	}  

	/** 
	 * The width of the string drawn with the given font assuming 1px spacing between characters.
     */
	static uint8_t textWidth(Data font, const char *text) {

		char ch;
		const char *src = text;
		uint8_t result = 0;
		while ((ch = *src++)) {      
			uint8_t width = dataForCharacter(font, ch, NULL);
			result += width + 1;
		}

		return result;
	}
	
	/** 
	 * Returns how many characters will fit max_width pixels without clipping. 
	 */
	static uint8_t numberOfCharsFittingWidth(Data font, const char *text, uint8_t max_width, uint8_t *actual_width) {
  
		uint8_t result = 0;

		char ch;
		const char *src = text;
		uint8_t total_width = 0;
		while ((ch = *src++)) {
			
			uint8_t new_total_width = total_width + dataForCharacter(font, ch, NULL) + 1;
			
			if (new_total_width > max_width)
				break;
			
			total_width = new_total_width;
			result++;
		}

		return result;
	}
	
	typedef enum : uint8_t {
		DrawingScale1 = 1,
		DrawingScale2,
		DrawingScale3,
		DrawingScale4
	} DrawingScale;
		
	/** 
	 * Renders a text string with the given font by sending the corresponding bytes directly to the LCD 
	 * supporting Display8RowOutput protocol (see the corresponding prototype).
	 * The `max_width` tells how many bytes we are allowed to output.
	 * The `xor_mask` is XORed with every character and when set to 0xFF or 0x7E can be used to render inverted text.
	 */
	template<class MonochromeDisplayPageOutput>
	static uint8_t draw(
		Data font, 
	 	uint8_t col,
		uint8_t page,
		uint8_t max_width, 
		const char *text, 
		DrawingScale scale = DrawingScale1,
		uint8_t xor_mask = 0
	) {		
		uint8_t result;
		for (uint8_t phase = 0; phase < scale; phase++) {
			result = drawPhase<MonochromeDisplayPageOutput>(phase, scale, font, col, page, max_width, text, xor_mask);
		}
		return result;
	}   

protected:
	
	template<uint8_t bit, uint8_t phase, uint8_t scale>
	static inline uint8_t stretched(uint8_t b) {
		const uint8_t src_bit = (bit + phase * 8) / scale;
		return ((b & (1 << src_bit)) >> src_bit) << bit;
	}
	
	template<uint8_t phase, uint8_t scale>
	static inline uint8_t stretchedByte(uint8_t b) {
		return stretched<0, phase, scale>(b)
			| stretched<1, phase, scale>(b)
			| stretched<2, phase, scale>(b)
			| stretched<3, phase, scale>(b)
			| stretched<4, phase, scale>(b)
			| stretched<5, phase, scale>(b)
			| stretched<6, phase, scale>(b)
			| stretched<7, phase, scale>(b);
	}	
		
	static uint8_t scaledByte(uint8_t phase, DrawingScale scale, uint8_t b) {
		
		switch (scale) {
			case DrawingScale1:
				return b;
			case DrawingScale2:
				if (phase == 0) {
					return stretchedByte<0, 2>(b);
				} else {
					return stretchedByte<1, 2>(b);
				}
			case DrawingScale3:
				if (phase == 0) {
					return stretchedByte<0, 3>(b);
				} else if (phase == 1) {
					return stretchedByte<1, 3>(b);
				} else {
					return stretchedByte<2, 3>(b);
				}
		}
		return b;
	}
   
	template<class MonochromeDisplayPageOutput>
	static uint8_t drawPhase(
		uint8_t phase,
		DrawingScale scale,
		Data font, 
	 	uint8_t col,
		uint8_t page,
		uint8_t max_width, 
		const char *text,
		uint8_t xor_mask
	) {
		MonochromeDisplayPageOutput::beginWritingPage(col, page + phase);

		char ch;
		const char *src = text;

		uint8_t width_left = max_width;
		
		uint8_t spacing_byte = scaledByte(phase, scale, xor_mask);

		while ((ch = *src++)) {

			uint8_t bitmap[8];
			uint8_t width = dataForCharacter(font, ch, bitmap);

			for (uint8_t i = 0; i < width; i++) {
				for (uint8_t j = 0; j < scale; j++) {
					MonochromeDisplayPageOutput::writePageByte(scaledByte(phase, scale, bitmap[i] ^ xor_mask));
					if (--width_left == 0) {
						MonochromeDisplayPageOutput::endWritingPage();
						return 0;
					}
				}
			}

			for (uint8_t j = 0; j < scale; j++) {
				MonochromeDisplayPageOutput::writePageByte(spacing_byte);
				if (--width_left == 0) {
					break;
				}
			}
		}
	
		MonochromeDisplayPageOutput::endWritingPage();

		return max_width - width_left;
	}   
   
public:

	template<class MonochromeDisplayPageOutput>
	static uint8_t drawCentered(
		Data font, 
	 	uint8_t col,
		uint8_t page,
		uint8_t max_width,
		const char *text, 
		DrawingScale scale = DrawingScale1,
		const uint8_t xor_mask = 0
	) {
		
		uint8_t len = 0;
		uint8_t w = 0;

		char ch;
		const char *src = text;
		while ((ch = *src++)) {
			
			uint8_t nw = w + scale * (dataForCharacter(font, ch, NULL) + 1);
			
			if (nw > max_width)
				break;
			
			w = nw;
			len++;
		}
		
		draw<MonochromeDisplayPageOutput>(font, col + (max_width - w) / 2, page, w, text, scale, xor_mask);
	}   
};

} // namespace

  