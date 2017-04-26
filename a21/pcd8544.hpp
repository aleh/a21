//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#include <Arduino.h>
#include <util/delay.h>

#include "pcd8544fonts.hpp"

namespace a21 {

/** 
 * Basic wrapper for a PCD8544 LCD display (such as the one that was used on Nokia 5110) with softwate SPI.
 * Note that the 'slow' parameter should be true only if this happen to be ported on a faster MCU 
 * and we need to ensure clock speed is less than 4 MHz. 
 * (The clock frequency will be less than 2 MHz on 16MHz Arduinos even with 'slow' being false.)
 */
template<typename pinRST, typename pinCE, typename pinDC, typename pinDIN, typename pinCLK, bool slow = false>
class PCD8544 {
  	  
  /** Delays execution for at least a half of the minimal clock period, which is 1/4 us in the datasheet. */
  static inline void delay125() __attribute__((always_inline)) {
	  _delay_us((1.0 / 4000000) / 2);
  }

  static inline void writeBit(bool b) __attribute__((always_inline)) {
    
    // Set the data bit, will have enough time to settle before the clock raising.
    pinDIN::write(b);

    // Clock the data bit out on the raising edge.
    pinCLK::setLow();    

    // Let's delay a bit in the slow mode.
    if (slow)
      delay125();
    
    // Clock it out!
    pinCLK::setHigh();
    
    // The preparation for the next bit slow enough, so no delays are actually needed here.
  }  
  
  enum ValueType : uint8_t {
    Command,
    Data
  };
  
  static void write(ValueType valueType, uint8_t value) {

    pinDC::write(valueType == Data);
    writeBit(value & _BV(7));
    writeBit(value & _BV(6));
    writeBit(value & _BV(5));
    writeBit(value & _BV(4));
    writeBit(value & _BV(3));
    writeBit(value & _BV(2));
    writeBit(value & _BV(1));
    writeBit(value & _BV(0));
  }  

  static void beginWriting() {
    pinCLK::setLow();
    pinCE::setLow();
  }
  
  static void endWriting() {
    pinCE::setHigh();
  }
          
  enum FunctionSetCommand : uint8_t {
    FunctionSet = 0x20,
    /** 0 - basic command set, 1 - extended command set. */
    H = 1,
    /** 0 - horizontal addressing mode, 1 - vertical addressing mode. */
    V = 2,
    /** 0 - chip active, 1 - chip in power down mode. */
    PD = 4
  };

  enum DisplayControlCommand : uint8_t {
    DisplayControl = 0x08,
    D = 0x04,
    E = 0x01,
    DisplayBlank = (uint8_t)(~(D | E)),
    NormalMode = (uint8_t)(D & ~E),
    AllSegmentsOn = (uint8_t)(~D & E),
    InverseVideoMode = (uint8_t)(D | E)
  };

  enum SetXAddressCommand : uint8_t {
    SetXAddress = 0x80,
    SetXAddressMask = 0x7F
  };
  
  enum SetYAddressCommand : uint8_t {
    SetYAddress = 0x40,
    SetYAddressMask = 0x07
  };

  enum TemperatureControlExtendedCommand : uint8_t {
    TemperatureControl = 0x04,
    TemperatureControlMask = 0x3
  };

  enum BiasSystemExtendedCommand : uint8_t {
    BiasSystem = 0x10,
    BiasSystemMask = 0x07
  };

  enum SetVopExtendedCommand : uint8_t {
    SetVop = 0x80,
    SetVopMask = 0x7F
  };

  static inline void extendedCommandSet(bool extended) {
    write(Command, extended ? (FunctionSet | H) & ~(PD | V) : (FunctionSet | 0) & ~(PD | V | H));
  }
  
  static inline void setAddressInternal(uint8_t col, uint8_t row) {
    extendedCommandSet(false);
    write(Command, SetXAddress | col);
    write(Command, SetYAddress | row);
  }
  
  static inline void config(uint8_t operatingVoltage, uint8_t biasSystem, uint8_t temperatureControl) {
    
    beginWriting();

    // TODO: perhaps make the display mode a parameter?
    extendedCommandSet(false);
    write(Command, DisplayControl | NormalMode);
        
    extendedCommandSet(true);    
    write(Command, SetVop | (operatingVoltage & SetVopMask));
    write(Command, BiasSystem | (biasSystem & BiasSystemMask));
    write(Command, TemperatureControl | (temperatureControl & TemperatureControlMask));
            
    endWriting();
  }  
    
public:
  
  /** Number of addressable rows, with every row corresponding to 8 horizontal lines of actual pixels. */
  static const int Rows = 6;
  
  /** Number of addressable columns, though unlike rows every column corresponds to 1 vertical line of pixels. */
  static const int Cols = 84;
  
  /** Maximum value for the parameter of operatingVoltage function, though the actual usable values are usually much smaller. */
  const uint8_t MaxVoltage = 0x7F;  

  /** Sets operational voltage affecting contrast of the display. */
  static void operatingVoltage(uint8_t value) {
    beginWriting();
    extendedCommandSet(true);
    write(Command, SetVop | value);
    endWriting();
  }

  /** Clears the display. */
  static void clear() {
    beginWriting();
    setAddressInternal(0, 0);
    for (int i = 0; i < Rows * Cols; i++) {
      write(Data, 0);
    }
    endWriting();
  }
    
  /** Initializes the device and transport pins. */
  static void begin(uint8_t operatingVoltage = 52, uint8_t biasSystem = 4, uint8_t temperatureControl = 2) {
    
    pinDIN::setOutput();
    pinDIN::setLow();
    
    pinCLK::setOutput();
    pinCLK::setLow();
    
    pinDC::setOutput();
    pinDC::setLow();
        
    pinCE::setOutput();
    pinCE::setHigh();

    pinRST::setOutput();    
    pinRST::setLow();
    delay125();
    pinRST::setHigh();

    config(operatingVoltage, biasSystem, temperatureControl);
    
    clear();
  }
  
  /** 
   * Transports a bunch of bytes for the given row. Each byte is responsible for a 8 pixel column within the row 
   * (MSB is in the bottom of the row, LSB is in the top).
   *
   * The row is filled from left to right (i.e. column address is automaticallt incremented).
   *                    col      col + 1
   *  line row * 8:     # bit 0  # bit 0  ...
   *  line row * 8 + 1: # bit 1  # bit 1  ...
   *     ...
   *  line row * 8 + 7: # bit 7  # bit 7  ...
   *                    ^        ^
   *                    byte 0   byte 1
   *
   * Note that if more bytes are provided than is left in the row, then they'll be written to the next one 
   * (or the first one in case of the last row). 
   */
  static void writeRow(uint8_t col, uint8_t row, const uint8_t *data, uint8_t data_length) {
    beginWriting();
    setAddressInternal(col, row);
    const uint8_t *src = data;
    for (uint8_t c = data_length; c > 0; c--) {
      write(Data, *src++);
    }
    endWriting();
  }
  
  static void fillRow(uint8_t col, uint8_t row, uint8_t length, uint8_t filler) {
    beginWriting();
    setAddressInternal(col, row);
    for (uint8_t c = length; c > 0; c--) {
      write(Data, filler);
    }
    endWriting();
  }
  
  //
  // Fonts
  //
  /** Returns the width of the glyph corresponding to a character in the given font and, if a buffer is provided, 
  * copies glyph's bitmap into it. */
  static uint8_t dataForCharacter(PCD8544Font font, char ch, uint8_t *buffer) {
 
    const uint8_t *p = font;

    uint8_t options = pgm_read_byte(p++);
    if ((options & 1) && 'a' <= ch && ch <= 'z') {
      ch = ch - 'a' + 'A';
    }

    while (true) {

      // First character in the range (0 would mean no more ranges are defined).
      uint8_t first = pgm_read_byte(p++);
      if (first == 0) 
        break;

      // Last character in the range.
      uint8_t last = pgm_read_byte(p++);

      // Number of bytes every character in the range occupies. 
      uint8_t bytes_per_character = pgm_read_byte(p++);

      if (first <= ch && ch <= last) {
        
        // Our character is in the range, let's copy the data for it.
        
        p += (ch - first) * bytes_per_character;
        // The first byte of the glyph data is the actual width of the glyph.
        uint8_t width = pgm_read_byte(p++);
        // Copy the bitmap if the caller expects it.
        if (buffer) {
          memcpy_PF(buffer, (uint_farptr_t)p, width);
        }

        return width;
      }

      // Well, let's skip to the next range of characters.
      p += (last + 1 - first) * bytes_per_character;      
    }

    // Let's return data for sort of a default character.
    return dataForCharacter(font, '?', buffer);
  }  

  static uint8_t textWidth(PCD8544Font font, const char *text) {
    
    char ch;
    const char *src = text;
    uint8_t result = 0;
    while ((ch = *src++)) {      
        uint8_t width = dataForCharacter(font, ch, NULL);
        result += width + 1;
    }
    
    return result;
  }
  
  /** Returns how many characters will fit max_width without being truncacted. */
  static uint8_t numberOfCharsFittingWidth(PCD8544Font font, const char *text, uint8_t max_width) {
    
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
  
  static uint8_t drawText(
    PCD8544Font font, 
    const uint8_t col, 
    const uint8_t row, 
    const uint8_t max_width, 
    const char *text, 
    const uint8_t xor_mask = 0
  ) {
    
    beginWriting();
    
    setAddressInternal(col, row);
    
    char ch;
    const char *src = text;
    uint8_t width_left = max_width;
    while ((ch = *src++)) {
      
        uint8_t bitmap[8];
        uint8_t width = dataForCharacter(font, ch, bitmap);
        
        for (uint8_t i = 0; i < width; i++) {
          write(Data, bitmap[i] ^ xor_mask);
          if (--width_left == 0) {
            endWriting();
            return 0;
          }
        }
        
        write(Data, xor_mask);
        if (--width_left == 0)
          break;
    }
    
    endWriting();
    
    return max_width - width_left;
  }
};

template<typename lcd, typename font>
class PCD8544Console {
  
private:
  
  static const uint8_t MaxCols = lcd::Cols / 4;
  
  char _buffer[lcd::Rows][MaxCols + 1];
  uint8_t _row;
  uint8_t _col;
  uint8_t _rowWidth;
  uint8_t _filledRows;
  
  void newline() {
    
    _col = 0;
    _rowWidth = 0;
    
    _row++;
    if (_row >= lcd::Rows) {
      _row = 0;
    }
    _filledRows++;
    if (_filledRows >= lcd::Rows) {
      _filledRows--;
    }
    _buffer[_row][_col] = 0;
  }
  
  void carriage_return() {
    _col = 0;
    _rowWidth = 0;
  }

public:
  
  PCD8544Console() : _row(0), _col(0), _rowWidth(0), _filledRows(0) {}
  
  void clear() {
    _row = _filledRows = 0;
    _col = 0;
    _rowWidth = 0;
    for (uint8_t row = 0; row < lcd::Rows; row++) {
      _buffer[row][0] = 0;
    }
  }
  
  void draw() {
    
    for (uint8_t i = 0; i < lcd::Rows; i++) {
      
      int8_t row_index = _row - _filledRows + i;
      if (row_index < 0)
        row_index += lcd::Rows;
      
      // Print the row and erase the rest.
      uint8_t width = lcd::drawText(font::font(), 0, i, lcd::Cols, _buffer[row_index]);
      lcd::fillRow(width, i, lcd::Cols - width, 0);
    }
  }

  void print(const char *text) {
    
    char ch;
    const char *src = text;
    while ((ch = *src++)) {
      if (ch >= ' ') {
      
        uint8_t width = lcd::dataForCharacter(font::font(), ch, NULL);
        if (_col >= MaxCols || _rowWidth + width >= lcd::Cols) {
          newline();        
        }
    
        _buffer[_row][_col] = ch;
        _col++;
        _buffer[_row][_col] = 0;
        _rowWidth += width + 1;
      
      } else if (ch == '\n') {
        newline();
      } else if (ch == '\r') {
        carriage_return();
      }
    }
  }    
};

} // namespace
