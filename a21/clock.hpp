//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>

#include "pins.hpp"

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
  
  static inline void delayMicroseconds(uint16_t us) {
    ::delayMicroseconds(us);
  }
}; 
  
} // namespace
