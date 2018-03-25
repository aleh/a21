//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>
#include <a21/clock.hpp>

namespace a21 {

/**
 * Software serial port, 8-N-1, TX only. 
 */
template<typename pinTX, unsigned long baudRate, typename Clock = ArduinoClock>
class SerialTx {
  
private:
    
  static inline void writeBit(bool b) __attribute__((always_inline)) {
    pinTX::write(b);
    Clock::delayMicroseconds(1000000.0 * (1.0 / baudRate - 5.0 / F_CPU));
  }  
  
public:
  
  static void begin() {
    pinTX::setOutput();
    pinTX::setHigh();
  }
  
  static void write(uint8_t value) {
    noInterrupts();
    writeBit(false);
    writeBit(value & _BV(0));
    writeBit(value & _BV(1));
    writeBit(value & _BV(2));
    writeBit(value & _BV(3));
    writeBit(value & _BV(4));
    writeBit(value & _BV(5));
    writeBit(value & _BV(6));
    writeBit(value & _BV(7));
    writeBit(true);
    interrupts();
  }
};
  
} // namespace
