//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>

/**
 * Monochrome framebuffer with layout compatible to monochrome LCDs like PCD8544 (from Nokia 3310). 
 * The `display` class should support a single static method helping to transfer the framebuffer:
 * static void writeRow(uint8_t col, uint8_t row, const uint8_t *data, uint16_t data_length)
 */
template<uint8_t _rows, uint8_t _cols, typename _display>
class Framebuffer {
private:

  int8_t _translationY;

  inline uint8_t clamp(int8_t value, uint8_t max) __attribute__((always_inline)) {
    if (value < 0)
      return 0;
    else if (value >= max)
      return max - 1;
    else
      return value;
  }
  
  /** This is used to shift all the drawing operations with "tile-based" rendering. */
  void setTranslation(int8_t rows) {
    _translationY = rows * 8;
  }  
    
public:
    
  static const uint8_t Cols = _cols;
  static const uint8_t Rows = _rows;
  static const uint8_t Width = _cols;
  static const uint8_t Height = _rows * 8;

  /** The actual framebuffer can be accessed directly. */
  uint8_t data[Cols * Rows];
        
  /** Tile based rendering: the given drawing routine is called multiple times to render a part of the whole picture
   * matching dimensions of the framebuffer; after each drawing a tile the framebuffer is flushed to the display. */
  void draw(void (*draw)(Framebuffer& fb)) {

    uint8_t row;
    for (row = 0; row + Rows <= _display::Rows; row += Rows) {      
      setTranslation(row);
      draw(*this);
      _display::writeRow(0, row, data, sizeof(data));
    }
    
    if (row < _display::Rows) {
      setTranslation(row);
      draw(*this);
      _display::writeRow(0, row, data, (_display::Rows - row) * Cols);
    }
  }
  
  void blit(int8_t x, int8_t y, const uint8_t *bitmap) {
    
    int8_t yy = y - _translationY;
    
    const uint8_t *src = bitmap;
    uint8_t width = *src++;
    uint8_t height = *src++;
        
    if (x + width <= 0 || x >= Width || yy + height <= 0 || yy >= Height) {
      return;
    }
    
  }
  
  void line(int8_t x1, int8_t y1, int8_t x2, int8_t y2, uint8_t color) {
      
  }
  
  /** Fills the framebuffer with a specified color. */
  void clear(uint8_t color) {
    memset(data, color ? 0xFF : 0x00, sizeof(data));
  }
  
  /** Draws a rectangle with the top left corner at the given point having the given size. */
  void drawRect(int8_t x, int8_t y, uint8_t width, uint8_t height, uint8_t color) {
    
    if (width == 0 || height == 0)
      return;
    
    // Letting the lines overlap at the corners, should be fast enough.
    drawHorizontalLine(x, y, width, color);
    drawHorizontalLine(x, y + height - 1, width, color);
    drawVerticalLine(x, y, height, color);
    drawVerticalLine(x + width - 1, y, height, color);
  }
  
  /** Draws a vertical line beginning at the given point and having the length specified. 
   * This should be faster than a generic line drawing routine. 
   * It can be made even faster by setting 8 bits at a time, but the code would become larger. */
  void drawVerticalLine(int8_t x, int8_t y, uint8_t length, uint8_t color) {
    
    if (x < 0 || x >= Width)
      return;
    
    int8_t yy1 = y - _translationY;
    int8_t yy2 = yy1 + length - 1;
    
    if (yy2 < 0 || yy1 >= Height)
      return;
    
    if (yy1 < 0)
      yy1 = 0;
    if (yy2 > Height - 1)
      yy2 = Height - 1;
    
    uint8_t *dst = data + (yy1 >> 3) * Cols + x;
    uint8_t mask = 1 << (yy1 & 7);
    
    if (color) {
      for (uint8_t yy = yy1; yy <= yy2;) {
        do {
          *dst |= mask;
          mask <<= 1;
          yy++;
        } while (mask != 0 && yy <= yy2);
        dst += Cols;
        mask = 1;
      }
    } else {
      mask = ~mask;
      for (uint8_t yy = yy1; yy <= yy2; y++) {
        *dst &= mask;
        dst += Cols;
        mask = (mask == 0x7F) ? 0xFE : (mask << 1) | 1;
      }
    }
  }
  
  /** Draws a horizontal line beginning at the given point and having the specified length.
   * This should be faster than a generic line drawing routine. */
  void drawHorizontalLine(int8_t x, int8_t y, uint8_t length, uint8_t color) {
    
    int8_t yy = y - _translationY;
      
    if (yy < 0 || yy >= Height)
      return;
    
    int8_t xx1 = x;
    int8_t xx2 = x + length - 1;        
    if (xx2 < 0 || xx1 >= Width)
      return;
    
    if (xx1 < 0)
      xx1 = 0;
    if (xx2 > Width - 1)
      xx2 = Width - 1;
    
    uint8_t *dst = data + (yy >> 3) * Cols + xx1;
    uint8_t mask = 1 << (y & 7);
    
    if (color) {
      for (uint8_t xx = xx1; xx <= xx2; xx++) {
        *dst++ |= mask;
      }
    } else {
      mask = ~mask;
      for (uint8_t xx = xx1; xx <= xx2; xx++) {
        *dst++ &= mask;
      }
    }
  }
};
