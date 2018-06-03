//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

namespace a21 {
  
template<typename i2c, uint8_t slave_address = 0x3C>
class SSD1306 {
public:

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
  
  /** Ends the sequence of command bytes started via beginCommand(). */
  static inline bool endCommand() {
    i2c::stop();
    return true;
  }
  
  /** Ends the sequence of data bytes started via beginData(). */
  static inline bool endData() {
    i2c::stop();
    return true;
  }  

  /** @{ */
  /** Shortcuts for 1 to 3 byte commands, replacing beginCommand()/write(a1)..write(a3)/endCommand() sequences. */
  
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

  /** A shortcut transferring a single data byte to a location determined by the current addressing mode and row/column addresses. */
  static inline bool writeData(uint8_t a) {
    return beginData() && i2c::write(a) && endData();
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
  
  enum AddressingMode : uint8_t {
    AddressingModeHorizontal = 0,
    AddressingModeVertical = 1,
    AddressingModePage = 2
  };
    
  static inline bool setAddressingMode(AddressingMode mode) {
    // "Set Memory Addressing Mode".
    return writeCommand(0x20, mode);
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
  
  /** Most likely you need to enable this by default, because when not "zoomed in" the display downscales
   * the contents of the memory vertically twice, i.e. displays every second row. */
  static inline bool setZoomInEnabled(bool enabled) {
    // "Set Zoom In" command.
    return writeCommand(0xD6, enabled ? 1 : 0);
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
 /** Horizontal or vertical addressing mode. */

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
 
 /** Allows to vertically offset the contents being displayed. Can be nice for hardware-assisted vertical scrolling. 
  * The value should be from 0-63 range. */
 static inline bool setDisplayStartLine(uint8_t value) {
   return writeCommand(0x40 | value & 0x3F);
 }
 
 /** Allows to flip the output vertically. Handy when the display is mounted upside down. */
 static inline bool setFlippedVertically(bool flipped) {
   return writeCommand(flipped ? 0xC8 : 0xC0);
 }
 
};

};