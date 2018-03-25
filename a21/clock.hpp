//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>

#if defined(ARDUINO_ARCH_AVR)
#include <util/delay.h>
#endif

namespace a21 { 

/** 
 * Wrapping clock-related functions so it is possible to switch to alternative clock implementations later.
 */
class ArduinoClock {
  
public:
  
  static inline uint8_t micros8() __attribute__((always_inline)) {
    return (uint8_t)::micros();
  }
  
  static inline uint16_t micros16() __attribute__((always_inline)) {
    return (uint16_t)::micros();
  }
  
  static inline void delay(uint16_t ms) {
    ::delay(ms);
  }
  
  /** Busy-waits for the specified number of microseconds. */
  static inline void delayMicroseconds(double us) __attribute__((always_inline)) {
    // Don't delay if it's going to be less than a single clock cycle.
    #if defined(ARDUINO_ARCH_AVR)
    _delay_us(us);
    #else
    if (us > 0.5 * 1000000.0 / F_CPU) {
      ::delayMicroseconds(us);
    }
    #endif
  }
}; 
  
} // namespace
