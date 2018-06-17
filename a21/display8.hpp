//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <a21/print.hpp>

namespace a21 {
	
/** 
 * This is to informally describe an interface of a monochrome LCD having "one byte per 8-pixel column of a page"
 * layout and supporting direct output to its memory. 
 *
 * The layout mentioned can be can be depicted like that:
 * \verbatim
 *           C C       C                    
 *           O O  ...  O                    
 *           L L       L                    
 *           0 1       N                    
 *          ┌─┬─┬─────┬─┐                   
 *          │0│0│     │0│ ROW P * 8         
 *          │1│1│     │1│ ROW P * 8 + 1     
 *          │2│2│     │2│                   
 *   PAGE P │3│3│ ... │3│                   
 *          │4│4│     │4│                   
 *          │5│5│     │5│                   
 *          │6│6│     │6│                   
 *          │7│7│     │7│ ROW P * 8 + 7     
 *          └─┴─┴─────┴─┘                   
 * \endverbatim
 *
 * (The "page" terminology is coming from the datasheet of SSD1306 and I think it's better to use it for other displays 
 * instead of a "row", which could be confusing.)
 */
template<typename dummy>
class MonochromeDisplayPageOutput {
public:
	
	/** You pass page number and the start colum the writing will begin with. 
	 * Note the order of parameters still corresponds to the usual X/Y order. */
	static void beginWritingPage(uint8_t col, uint8_t page) {}
	
	/** Then you send all the bytes you need. No clipping occurs, so the last column should be specified elsewhere. */
	static void writePageByte(uint8_t b) {}
	
	/** And don't forget to call this to finish the transfer. */
	static void endWritingPage() {}
	
	/** 
	 * Transports a bunch of bytes for the given page. 
	 * The layout directly corresponds with the memory layout of the LCD, see above.
	 */
	static void writePage(uint8_t col, uint8_t page, const uint8_t *data, uint16_t data_length) {}
	 
	/** Similar to writePage, but the same byte is sent `length` times. */
	static void fillPage(uint8_t col, uint8_t page, uint8_t filler, uint8_t length) {}		
};

/** 
 * This is to document a minimal interface of a monochrome display supporting paged ccess. 
 */
template<typename dummy>
class Display8 {
	
	static const uint8_t Pages = 8;
	static const uint8_t Rows = 8 * Pages;
	static const uint8_t Cols = 128;
	
	// Should support MonochromeDisplayPageOutput.
};

/**
 * Turns a monochrome LCD supporting simple text output into a simple text-only display with autoscrolling ("console").
 * Note that we don't inherit Arduino's Print class to keep the compiled code size small.
 * TODO: use the Print<> template for the printing functions
 */
template<typename lcd, typename font = Font8Console>
class Display8Console : public Print< Display8Console<lcd, font> > {
  
private:
  
	// We assume that every character occupies at least 4px.
	static const uint8_t MaxCols = lcd::Cols / 4;

	char _buffer[lcd::Pages][MaxCols + 1];
	uint8_t _row;
	uint8_t _col;
	uint8_t _rowWidth;
	uint8_t _filledRows;
	bool _dirty;
  
	void _lf() {

		_col = 0;
		_rowWidth = 0;

		_row++;
		if (_row >= lcd::Pages) {
			_row = 0;
		}
		
		_filledRows++;
		if (_filledRows >= lcd::Pages) {
			_filledRows--;
		}
		
		_buffer[_row][_col] = 0;
	}
  
	void cr() {
		_col = 0;
		_rowWidth = 0;
	}
  
	void _clear() {
		_row = _filledRows = 0;
		_col = 0;
		_rowWidth = 0;
		for (uint8_t row = 0; row < lcd::Pages; row++) {
			_buffer[row][0] = 0;
		}
		_dirty = true;    
	}
	
	void _draw() {

		if (!_dirty)
			return;

		_dirty = false;

		for (uint8_t i = 0; i < lcd::Pages; i++) {

			int8_t row_index = _row - _filledRows + i;
			if (row_index < 0)
				row_index += lcd::Pages;

			// Print the row and erase the space after the last character.
			uint8_t width = lcd::drawText(font::data(), 0, i, _buffer[row_index]);
			lcd::fillPage(width, i, 0, lcd::Cols - width);
		}
	}

	void _write(char ch) {

		if (ch >= ' ') {

			uint8_t width = Font8::dataForCharacter(font::data(), ch, NULL);
			if (_col >= MaxCols || _rowWidth + width >= lcd::Cols) {
				_lf();        
			}

			_buffer[_row][_col] = ch;
			_col++;
			_buffer[_row][_col] = 0;
			_rowWidth += width + 1;

			} else if (ch == '\n') {
				_lf();
			} else if (ch == '\r') {

			cr();      
		}

		_dirty = true;
	}  

	typedef Display8Console<lcd, font> Self;
	
	static Self& getSelf() {
		static Self self = Display8Console();
		return self;
	}

protected:
	
	friend Print< Display8Console<lcd, font> >;
	
	static void lf() {
		getSelf()._lf();
	}
		
	/** Prints a single character. This is what the Print<> template is using for output. */
	static void write(char ch) {
		getSelf()._write(ch);
	}
  
public:	
  
	Display8Console() 
		: _row(0), _col(0), _rowWidth(0), _filledRows(0)
	{}
	
	/** Clears the console without redrawing it on the LCD. */
	static void clear() {
		getSelf()._clear();
	}
	
	/** Transfers the contents of the console buffer to the LCD. 
	* Note that this is not called automatically for every print() function. */
	static void draw() {
		getSelf()._draw();
	}		
};

} // namespace
