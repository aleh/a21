//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>

#include <a21/pins.hpp>

#if defined(ARDUINO_ARCH_AVR)
#include <util/delay.h>
#endif

namespace a21 {

/**
 * Software SPI which can use FastPin templates.
 * Supports only sending the data out for now, not clocking it in.
 * Assumes that each bit is clocked on the rising edge of the clock and that CE pin is active LOW.
 */
template<typename pinMOSI, typename pinCLK, typename pinCE, unsigned long maxFrequency = 4000000>
class SPI {
  
private:
  
  /** Busy-waits for the specified number of microseconds. */
  static inline void delayMicroseconds(double us) __attribute__((always_inline)) {
    // Don't need a delay if it's going to be less than a clock cycle.
    if (us > 0.5 * 1000000.0 / F_CPU) {
      #if defined(ARDUINO_ARCH_AVR)
      _delay_us(us);
      #else
      delayMicroseconds(us);
      #endif
    }
  }
  
  static inline void writeBit(bool b) __attribute__((always_inline)) {
    
    // Set the data bit, will have enough time to settle before the clock is raised,
    // even with optimizations will take 5-6 cycles because of the branching involved.
    pinMOSI::write(b);
    
    // Will clock the data bit out on the raising edge, so let's make sure it's LOW now.
    // Will take at least 2 clock cycles.
    pinCLK::setLow();
    
    delayMicroseconds(1000000.0 * (0.5 / maxFrequency - (5.0) / F_CPU));

    // Clock it out!
    pinCLK::setHigh();

    delayMicroseconds(1000000.0 * (0.5 / maxFrequency - (2.0 + 2.0) / F_CPU));
  }  
  
public:
  
  /** Sets the mode for all the used pins. */
  static void begin() {
    pinMOSI::setOutput();
    pinMOSI::setLow();
    
    pinCLK::setOutput();
    pinCLK::setLow();
    
    pinCE::setOutput();
    pinCE::setHigh();    
  }

  /** Enables the slave by setting CE low. */
  static void beginWriting() {
    pinCLK::setLow();
    pinCE::setLow();
  }
  
  /** Clocks out a single byte on the MOSI line. */
  static void write(uint8_t value) {
    writeBit(value & _BV(7));
    writeBit(value & _BV(6));
    writeBit(value & _BV(5));
    writeBit(value & _BV(4));
    writeBit(value & _BV(3));
    writeBit(value & _BV(2));
    writeBit(value & _BV(1));
    writeBit(value & _BV(0));
  }  

  /** Disables the slave by setting CE high. */
  static void endWriting() {
    pinCE::setHigh();
  }  
};
  
} // namespace
