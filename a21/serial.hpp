//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>
#include <a21/clock.hpp>
#include <a21/print.hpp>

namespace a21 {

#pragma GCC optimize ("O2")

/**
 * Software serial port, 8-N-1, TX only. 
 */
template<typename pinTX, unsigned long baudRate, typename Clock = ArduinoClock>
class SerialTx : public Print< SerialTx<pinTX, baudRate, Clock> > {
  
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

/** 
 * Simple software serial port, 8-N-1, RX only.
 */
template<typename pinRX, unsigned long baudRate, typename Clock = ArduinoClock>
class SerialRx {
  
	static constexpr double oneBitDelayUs = 1000000.0 / baudRate;
  
  static inline void readNextBit(uint8_t& result) __attribute__((always_inline)) {
    result >>= 1;
    if (pinRX::read()) {
      result |= 0x80;
    } else {
      result |= 0x00;
    }
    Clock::delayMicroseconds(oneBitDelayUs);
  }
    
public:
  
  static void begin() {
    pinRX::setInput();
  }
  
  /** Tries to read the next byte on the pin. 
   * Zero is returned when no byte is available (don't see the start bit or could finish the reception). */
  static uint8_t read(uint8_t start_bit_timeout) {
        
    uint8_t result = 0;    
    uint8_t start_time = Clock::micros8();
    uint8_t t;
        
    while (true) {
      if (!pinRX::read()) {
        break;
      }
      if (Clock::micros8() - start_time >= start_bit_timeout) {
        return 0;
      }
    }
    
    cli();
            
    Clock::delayMicroseconds(1.1 * oneBitDelayUs);

    readNextBit(result);
    readNextBit(result);
    readNextBit(result);
    readNextBit(result);
    readNextBit(result);
    readNextBit(result);
    readNextBit(result);
    readNextBit(result);
    
    bool stop = pinRX::read();
    
    sei();
    
    return stop ? result : 0;
  }    
};

/** 
 * Software serial receiver that can be driven by a pin change interrupt.
 */
template<uint16_t baudRate>
class PinChangeSerialRx {
protected:
  
  /*~
  typedef PinChangeSerialRx<baudRate, timerTicksPerSecond> Self;
  
  static Self& getSelf() {
    static Self s = Self();
    return s;
  }
  
  // Bit we are on.
  enum State : uint8_t {
    BeforeStartBit,
    StartBit,
    Bit0,
    Bit1,
    Bit2,
    Bit3,
    Bit4,
    Bit5,
    Bit6,
    Bit7,
    Bit8,
    StopBit
  } state;
  
  // Timestamp of the last event, if any.
  uint16_t prevTime;
  bool prevValue;
  
public:

  static void begin() {
    getSelf().state = BeforeStartBit;
  }
  */
  /** Should be called every time the pin changes, possibily from an interrupt handler. */
  /*
  static void pinChange(uint16_t time, bool value) {
    Self& self = getSelf();
    if (self.state == BeforeStartBit) {
        if (value) {
          self.prevTime = time;
          self.state = StartBit;
        }
    } else {
      uint16_t t = time - prevTime;
      t += halfBitTime;
    }
  }
  */
  static void check(uint16_t time_us) {
  }
  
};


#pragma GCC reset_options
  
} // namespace
