//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include "font8.hpp"
#include "display8.hpp"

namespace a21 {
	  
/** 
 * Wrapper for I2C OLED displays based on SSD1306 chip.
 * 
 * "Pages" are groups of 8 rows where each byte of the page is responsible for 8 pixels of a corresponding column. 
 * The least significant bits of every byte in the page determine the contents of the topmost 1-pixel row.
 *
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
 */
template<
	typename i2c, 
	uint8_t pages = 8,
	uint8_t slave_address = 0x3C
>
class SSD1306 : public Display8< SSD1306<i2c, pages, slave_address> >{
	
public:
	
	static const uint8_t Pages = pages;
	static const uint8_t Rows = 8 * pages;
	static const uint8_t Cols = 128;
	
	typedef SSD1306<i2c, pages, slave_address> Self;

	/** @{ */
	/** Low-level command/data access. */

	/** Begins a sequence of command bytes. Must be paired with endCommand(). */
	static bool beginCommand() {
		return i2c::startWriting(slave_address) && i2c::write(0);
	}

	/** Begins a sequence of data bytes. Must be paired with endData(). */
	static bool beginData() {
		return i2c::startWriting(slave_address) && i2c::write(0x40);
	}  

	/** Writes a single data or command byte depending on the current mode. */
	static inline bool write(uint8_t a) {
		return i2c::write(a);
	}  

	/** Writes 2 data or command byte depending on the current mode. */
	static inline bool write(uint8_t a, uint8_t b) {
		return i2c::write(a) && i2c::write(b);
	}

	/** Writes 3 data or command byte depending on the current mode. */
	static inline bool write(uint8_t a, uint8_t b, uint8_t c) {
		return i2c::write(a) && i2c::write(b) && i2c::write(c);
	}

	/** Ends the sequence of command bytes started with beginCommand(). */
	static inline bool endCommand() {
		i2c::stop();
		return true;
	}

	/** Ends the sequence of data bytes started with beginData(). */
	static inline bool endData() {
		i2c::stop();
		return true;
	}  

	/** @{ */
	/** Shortcuts for 1-3 byte commands, replacing beginCommand()/write(a1)..write(a3)/endCommand() sequences. */

	static inline bool writeCommand(uint8_t a) {
		return beginCommand() && i2c::write(a) && endCommand();
	}    

	static inline bool writeCommand(uint8_t a, uint8_t b) {
		return beginCommand() && i2c::write(a) && i2c::write(b) && endCommand();
	}    

	static inline bool writeCommand(uint8_t a, uint8_t b, uint8_t c) {
		return beginCommand() && i2c::write(a) && i2c::write(b) && i2c::write(c) && endCommand();
	}
	
	/** @} */
	
	/** The simplest initialization sequence. Feel free to use your own instead. */
	static inline bool begin() {
		
		uint16_t tries = 0;
		
		// This seems to be the max start up time I observed.
		const uint16_t max_timeout_ms = 1500;
		
		// Assuming that for every attempt we have to transfer at least 10 bits (start condition, slave address 
		// and then acknowledgement bit) at I2C fast mode frequency of 400KHz.
		const uint16_t max_tries = 1 + max_timeout_ms * 400000L / (10 * 1000);
		
		while (tries++ <= max_tries) {
			if (available()) {
				return setZoomInEnabled(true) 
					&& setContrast(0);
			}
		}
		
		return false;
	}	
	
	/** Sends a NOP command and returns true if it was acknowledged. 
	 * Handy when checking if the display has finished its power on sequence and is ready to talk. */
	static inline bool available() {
		return writeCommand(0xE3); // NOP
	}

	/** Turns the display on without trying to reset it or set addressing modes, etc. */
	static inline bool turnOn() {
		return beginCommand() 
			&& write(0x8D, 0x14) // "Charge Pump Settings" command with "Enable charge pump during display on".
			&& write(0xAF) // "Set Display ON/OFF" command with X0 bit being "Display ON".
			&& endCommand();
	}
	
	/** Turns the display off. */
	static inline bool turnOff() {
		// "Set Display ON/OFF" command with X0 bit being "Display OFF".
		return writeCommand(0xAE);
	}

	/** Sets the contrast. The value can be between 0x00 and 0xFF. */
	static inline bool setContrast(uint8_t value) {
		// "Set Contrast Control" command.
		return writeCommand(0x81, value);
	}

	/** Allows to light up all the pixels regardless of the contents of the display's memory. 
	* Can be used for speciall effects, like quick flashes. */
	static inline bool setAllPixelsOn(bool enabled) {
		// "Entire Display On" command.
		return writeCommand(enabled ? 0xA5 : 0xA4);
	}

	/** Sets inverse mode, i.e. when zero pixels are lit up and ones are not. 
	* Can be used for convenience or special effects as well. */
	static inline bool setInverseMode(bool enabled) {
		// "Set Normal/Inverse Display" command.
		return writeCommand(enabled ? 0xA7 : 0xA6);
	}

	enum FadeMode {
		/** No fade in/out/blinking. */
		FadeModeDisabled = 0x00,
		/** The screen is fading out by changing the contrast. */
		FadeModeOut = 0x20,
		/** The screen is fading out and then in by changing the contrast. */
		FadeModeInOut = 0x30,    
		FadeModeBlink = FadeModeInOut
	};

	/** The `interval8` parameter defines how many frames to wait between every contrast changing step 
	* (the number of frames is `(interval8 + 1) * 8`). */
	static inline bool setFadeMode(FadeMode mode, uint8_t interval8) {
		// "Set Fade Out and Blinking" command.
		return writeCommand(0x23, mode | interval8);
	}

	/** Most likely you need to enable this by default on 128x32 version, because when not "zoomed in" 
	 * the display downscales the contents of the memory vertically twice, i.e. displays every second row. */
	static inline bool setZoomInEnabled(bool enabled) {
		// "Set Zoom In" command.
		return writeCommand(0xD6, enabled ? 1 : 0);
	}

	/** Addressing mode defines how the data bytes received will be laid in the display's memory. */
	enum AddressingMode : uint8_t {

		/**
		 * All the incoming bytes will be placed into the rectangle defined byt start/end columns 
		 * and start/end pages set via setColumnAddresses() and setPageAddresses() correspondingly. 
		 * In the horizontal addressing mode the rectangle will be filled in left-to-right/top-to-bottom order, 
		 * while in the vertical mode will be filled in top-to-bottom/left-to-right order.
		 * Any extra bytes will be again placed from the beginning of the rectangle. 
		 */
		AddressingModeHorizontal = 0,
		AddressingModeVertical = 1,
		
		/** 
		 * All the incoming bytes will be placed by the display into the page set via pageModeSetPage() 
		 * starting with the column set via pageModeSetStartColumn() and incrementing it further.
		 * After reaching the end column (which cannot be set it seems) it will restart at the column
		 * set via pageModeSetStartColumn() again and will stay on the same page.
		 */
		AddressingModePage = 2
	};

	static inline bool setAddressingMode(AddressingMode mode) {
		// "Set Memory Addressing Mode".
		return writeCommand(0x20, mode);
	}

	/** @{ */
	/** Page addressing mode. */

	/** Sets the start column for the page addressing mode. */
	static inline bool pageModeSetStartColumn(uint8_t col) {
		// "Set Lower Column Start Address for Page Addressing Mode" 
		// and "Set Higher Column Start Address for Page Addressing Mode" commands.
		return writeCommand(0x00 | (col & 0x0F), 0x10 | (col>> 4));
	}
	
	/** Sets the current page in page addressing mode. */
	static inline bool pageModeSetPage(uint8_t page) {
		// "Set Page Start Address for Page Addressing Mode" command.
		return writeCommand(0xB0 | page & 0x7);
	}

	/** @} */

	/** @{ */
	/** Horizontal or vertical addressing modes. */

	/** The columns have to be in 0-127 range, we don't mask or check them. */
	static inline bool setColumnAddresses(uint8_t start, uint8_t end) {
		// "Set Column Address" command.
		return writeCommand(0x21, start, end);
	}

	/** The page (row) address should be between 0 and 7, we don't check or mask them here. */
	static inline bool setPageAddresses(uint8_t start, uint8_t end) {
		// "Set Page Address" command.
		return writeCommand(0x22, start, end);
	}

	/** @} */

	/** 
	 * Allows to vertically offset the contents being displayed. 
	 * Can be nice for hardware-assisted vertical scrolling or double buffering. 
 	 * The value should be in the 0-63 range. 
	 */
	static inline bool setDisplayStartLine(uint8_t value) {
		return writeCommand(0x40 | value & 0x3F);
	}

	/** Allows to flip the output vertically. 
	 * Handy when the display is mounted upside down but we want to use the same addressing. */
	static inline bool setFlippedVertically(bool flipped) {
		return writeCommand(flipped ? 0xC8 : 0xC0) // "Set COM Output Scan Direction"
			&& writeCommand(flipped ? 0xA1 : 0xA0); // "Set Segment Re-map"
	} 

	/** @{ */
	/** Support for `MonochromeDisplayPageOutput`. */
	
	static inline void beginWritingPage(uint8_t col, uint8_t page) {
		setAddressingMode(AddressingModePage);
		pageModeSetPage(page);
		pageModeSetStartColumn(col);
		beginData();
	}
	
	static void writePageByte(uint8_t b) {
		write(b);
	}
	
	static void endWritingPage() {
		endData();
	}
	
	/** @} */
	
	/** @{ */
	/** Some basic drawing routines. See Display8 template. */ 
 
	/** @} */
};

}; // namespace

