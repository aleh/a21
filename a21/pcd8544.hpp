//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>

#include <a21/pcd8544fonts.hpp>
#include <a21/spi.hpp>
#include <a21/flashstring.hpp>

namespace a21 {
  
/** 
 * Basic wrapper for a PCD8544 LCD display (such as the one that was used on Nokia 5110) using software SPI.
 * The parameters are FastPin-wrapped pins in the order they have on the actual device (well, at least on mine):
 * RST, CE, DC, DIN, CLK.
 */
template<
   typename pinRST, typename pinCE, typename pinDC, typename pinDIN, typename pinCLK, 
   uint32_t maxFrequency = 4000000L
>
class PCD8544 {
  
public:
  
  enum Flags {
    InverseVideo,
    NormalVideo
  };
  
private:
  
  typedef SPI<pinDIN, pinCLK, pinCE, maxFrequency> spi;
  
  enum ValueType : uint8_t {
    Command,
    Data
  };
  
  static inline void write(ValueType valueType, uint8_t value) {
    pinDC::write(valueType == Data);
    spi::write(value);
  }  

  static inline void beginWriting() {
    spi::beginWriting();
  }
  
  static inline void endWriting() {
    spi::endWriting();
  }
          
  enum FunctionSetCommand : uint8_t {
    FunctionSet = 0x20,
    /** 0 - basic command set, 1 - extended command set. */
    FunctionSetH = 1,
    /** 0 - horizontal addressing mode, 1 - vertical addressing mode. */
    FunctionSetV = 2,
    /** 0 - chip active, 1 - chip in power down mode. */
    FunctionSetPD = 4
  };

  enum DisplayControlCommand : uint8_t {
    DisplayControl = 0x08,
    DisplayControlD = 0x04,
    E = 0x01,
    DisplayBlank = (uint8_t)(~(DisplayControlD | E)),
    NormalMode = (uint8_t)(DisplayControlD & ~E),
    AllSegmentsOn = (uint8_t)(~DisplayControlD & E),
    InverseVideoMode = (uint8_t)(DisplayControlD | E)
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
    write(
      Command, 
      extended ? (FunctionSet | FunctionSetH) & ~(FunctionSetPD | FunctionSetV) 
        : (FunctionSet | 0) & ~(FunctionSetPD | FunctionSetV | FunctionSetH)
    );
  }
  
  static inline void setAddressInternal(uint8_t col, uint8_t row) {
    // Assuming that we are not in the extended command set by default.
    //~ extendedCommandSet(false);
    write(Command, SetXAddress | col);
    write(Command, SetYAddress | row);
  }
  
  static inline void config(Flags flags, uint8_t operatingVoltage, uint8_t biasSystem, uint8_t temperatureControl) {
    
    beginWriting();
        
    extendedCommandSet(true);
    write(Command, SetVop | (operatingVoltage & SetVopMask));
    write(Command, BiasSystem | (biasSystem & BiasSystemMask));
    write(Command, TemperatureControl | (temperatureControl & TemperatureControlMask));
    
    extendedCommandSet(false);
    write(Command, DisplayControl | ((flags == InverseVideo) ? InverseVideoMode : NormalMode));
                
    endWriting();
  }  
    
public:
  
  /** Number of addressable rows, with every row corresponding to 8 horizontal lines of actual pixels. */
  static const uint8_t Rows = 6;
  
  /** Number of addressable columns, though unlike rows every column corresponds to 1 vertical line of pixels. */
  static const uint8_t Cols = 84;

  /** For convenience the width and the height of the display in pixels. */
  static const uint8_t Width = Cols;
  static const uint8_t Height = Rows * 8;
  
  /** Maximum value for the parameter of operatingVoltage function, though the actual usable values are usually much smaller. */
  static const uint8_t MaxVoltage = 0x7F;

  /** Sets operational voltage affecting the contrast of the display. */
  static void operatingVoltage(uint8_t value) {
    
    beginWriting();
    
    extendedCommandSet(true);
    write(Command, SetVop | value);
    
    // Always assuming that the normal command mode.
    extendedCommandSet(false);
    
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
      
  /** Initializes the display. */
  static void begin(Flags flags = NormalVideo, uint8_t operatingVoltage = 22, uint8_t biasSystem = 7, uint8_t temperatureControl = 2) {
    
    spi::begin();
        
    pinDC::setOutput();
    pinDC::setLow();

    pinRST::setOutput();    
    pinRST::setLow();
    // TODO: add delay template instead
    delayMicroseconds(1000000.0 / maxFrequency);
    pinRST::setHigh();

    config(flags, operatingVoltage, biasSystem, temperatureControl);
    
    clear();
  }
  
  /** 
   * Transports a bunch of bytes for the given row. The layout directly corresponds with the memory layout of the LCD, 
   * where each byte is responsible for a 8 pixel column within the row (MSB is in the bottom of the row, 
   * LSB is in the top).
   *
   * The row is filled from left to right (i.e. column address is automatically incremented).
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
  static void writeRow(uint8_t col, uint8_t row, const uint8_t *data, uint16_t data_length) {
    beginWriting();
    setAddressInternal(col, row);
    const uint8_t *src = data;
    for (uint16_t c = data_length; c > 0; c--) {
      write(Data, *src++);
    }
    endWriting();
  }

  /**
   * Similar to writeRow, but the same byte is sent `length` times.
   */
  static void fillRow(uint8_t col, uint8_t row, uint8_t filler, uint8_t length) {
    beginWriting();
    setAddressInternal(col, row);
    for (uint8_t c = length; c > 0; c--) {
      write(Data, filler);
    }
    endWriting();
  }
  
  //
  // Support for simple 8px high fonts fitting rows of the display exactly
  // TODO: move into its own template, can be used with other displays
  //
  
  /** 
   * Returns the width of the glyph corresponding to a character in the given font and, if a buffer is provided, 
   * copies glyph's bitmap bytes into it. 
   */
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

  /** The width of the string drawn with the given font. */
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
    
  /** 
   * Renders a text string by sending the corresponding bytes directly to the LCD.
   * The row and col parameters use LCD addressing system, where each rows occupies 8 pixels.
   * The max_width parameter is not clipped to the actual width available.
   * The xor_mask set to 0xFF or 0x7E can be used to render inverted text, for example.
   */
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
    
    // Well, let's clip it just in case.
    uint8_t m = Cols - col;
    uint8_t width_left = max_width < m ? max_width : m;
    
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
  
  /** Returns how many characters will fit max_width pixels without clipping. */
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
  
};

/**
 * Turns a PCD8544 LCD into a simple text-only display with autoscrolling.
 * Note that we don't inherit Arduino's Print class to keep the compiled code size small.
 * TODO: this was moved to Display8Console. Get rid of it here.
 */
template<typename lcd, typename font = PCD8544FontPixelstadTweaked>
class PCD8544Console {
  
private:
  
  static const uint8_t MaxCols = lcd::Cols / 4;
  
  char _buffer[lcd::Rows][MaxCols + 1];
  uint8_t _row;
  uint8_t _col;
  uint8_t _rowWidth;
  uint8_t _filledRows;
  bool _dirty;
  
  void lf() {
    
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
  
  void cr() {
    _col = 0;
    _rowWidth = 0;
  }
  
public:
  
  PCD8544Console() 
    : _row(0), _col(0), _rowWidth(0), _filledRows(0)
  {}
  
  /** Clears the console without redrawing it on the LCD. */
  void clear() {
    _row = _filledRows = 0;
    _col = 0;
    _rowWidth = 0;
    for (uint8_t row = 0; row < lcd::Rows; row++) {
      _buffer[row][0] = 0;
    }
    _dirty = true;    
  }
  
  /** Transfers the contents of the console buffer to the LCD. 
   * Note that this is not called automatically for every print() function. */
  void draw() {
    
    if (!_dirty)
      return;
    
    _dirty = false;
        
    for (uint8_t i = 0; i < lcd::Rows; i++) {
      
      int8_t row_index = _row - _filledRows + i;
      if (row_index < 0)
        row_index += lcd::Rows;
      
      // Print the row and erase the space after the last character.
      uint8_t width = lcd::drawText(font::font(), 0, i, lcd::Cols, _buffer[row_index]);
      lcd::fillRow(width, i, 0, lcd::Cols - width);
    }
  }

  void print(char ch) {
    
    if (ch >= ' ') {
    
      uint8_t width = lcd::dataForCharacter(font::font(), ch, NULL);
      if (_col >= MaxCols || _rowWidth + width >= lcd::Cols) {
        lf();        
      }
  
      _buffer[_row][_col] = ch;
      _col++;
      _buffer[_row][_col] = 0;
      _rowWidth += width + 1;
            
    } else if (ch == '\n') {
      
      lf();
      
    } else if (ch == '\r') {
      
      cr();      
    }
    
    _dirty = true;
  }
  
  void print(const char *str) {
    const char *src = str;
    char ch;
    while (ch = *src++) {
      print(ch);
    }
  }
  
  void print(FlashStringPtr str) {
    const char *src = (const char *)str;
    char ch;
    while (ch = pgm_read_byte(src++)) {
      print(ch);
    }
  }

  void print(int n) {
    char buf[5 + 2];
    itoa(n, buf, 10);
    print(buf);
  }
  
  void print(unsigned int n) {
    char buf[5 + 1];
    utoa(n, buf, 10);
    print(buf);
  }
    
  void print(long n) {
    char buf[10 + 2];
    ltoa(n, buf, 10);
    print(buf);
  }
  
  void print(unsigned long n) {
    char buf[10 + 1];
    ltoa(n, buf, 10);
    print(buf);
  }
    
  void println(const char *str) {
    print(str);
    lf();
  }
  
  void println(FlashStringPtr str) {
    print(str);
    lf();
  }  
  
  void println(int n) {
    print(n);
    lf();
  }
  
  void println(unsigned int n) {
    print(n);
    lf();
  }
  
  void println(long n) {
    print(n);
    lf();
  }  
  
  void println(unsigned long n) {
    print(n);
    lf();
  }  
  
  void println() {
    lf();
  }  
};

} // namespace
