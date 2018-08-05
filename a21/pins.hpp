//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <arduino.h>

namespace a21 {

/** 
 * Wrapper for a dummy pin.
 * This is handy to pass when a template requires one, but the pin is actually optional,
 * can be left unconnected or is driven by other circuit or user's code.
 */
template<bool value = false>
class UnusedPin {
public:

  static inline void setOutput() {}
  static inline void setInput(bool) { }
  
  static inline bool read() { return value; }
  
  static inline void setHigh() {}
  static inline void setLow() {}
  
  static inline void write(bool) { }
};

/** 
 * Wrapper for a pin that uses standard Arduino library calls (digitalRead and friends).
 * This is handy when debugging FastPin or on boards not supported by it.
 */
template<int pin>
class SlowPin {
public:
  
  static const int Pin = pin;

  static inline void setOutput() {
    pinMode(pin, OUTPUT);
  }
  
  static inline void setInput(bool pullup) {
    pinMode(pin, pullup ? INPUT_PULLUP : INPUT);
  }
  
  static inline bool read() {
    return digitalRead(pin) == HIGH;
  }
  
  static inline void setHigh() {
    digitalWrite(pin, HIGH);
  }
  
  static inline void setLow() {
    digitalWrite(pin, LOW);
  }

  static inline void write(bool b) {
    digitalWrite(pin, b ? HIGH : LOW);
  }
};

/** 
 * Wrapper for a pin that uses single instruction access to the corresponding port. 
 * Handy when a pin needs to be manipulated very quickly.
 * Pin numbers correspond to the numbers used by digitalWrite() on Arduino and compatible boards 
 * (though we support only ATmega and ATtiny boards here for now).
 */
template<int pin>
class FastPin {
  
public:
  static const int Pin = pin;  
  
#pragma GCC optimize ("O2")

// ATTiny85-based boards, like Digispark
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  
private:

  static uint8_t const mask = _BV(pin);
  
public:
	
  static inline void setOutput() __attribute__((always_inline)) {
    DDRB |= mask;
  }
  
  static inline void setInput(bool pullup) __attribute__((always_inline)) {
    DDRB &= ~mask;
    if (pullup) {
      PORTB |= mask;
    } else {
      PORTB &= ~mask;
    }      
  }
  
  static inline bool read() __attribute__((always_inline)) {
    return bit_is_set(PINB, pin);
  }
  
  static inline void setHigh() __attribute__((always_inline)) {
    PORTB |= mask;
  }
  
  static inline void setLow() __attribute__((always_inline)) {
    PORTB &= ~mask;
  }

  static inline void write(bool b) __attribute__((always_inline)) {
    if (b) {
      setHigh();
    } else {
      setLow();
    }
  }

// ATmega329-based boards, like Arduino Uno, Nano, or Yun
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__)

private:

  // Calculating our bitmask in one place. Unfortunately cannot make this trick for the port registers it seems.
  static uint8_t const mask = (pin < 8) ? _BV(pin) : _BV(pin - 8);
  
public:
	
  // Don't worry about all the if statements, they'll be optimized away as the expression is known at compilation time.
  
  static inline void setOutput() __attribute__((always_inline)) {
    if (pin < 8) {
      DDRD |= mask;
    } else {
      DDRB |= mask;
    }
  }
  
  static inline void setInput(bool pullup) __attribute__((always_inline)) {
    if (pin < 8) {
      DDRD &= ~mask;
      if (pullup) {
        PORTD |= mask;
      } else {
        PORTD &= ~mask;
      }
    } else {
      DDRB &= ~mask;
      if (pullup) {
        PORTB |= mask;
      } else {
        PORTB &= ~mask;
      }      
    }
  }
  
  static inline bool read() __attribute__((always_inline)) {
    if (pin < 8) {
      return bit_is_set(PIND, pin);
    } else {
      return bit_is_set(PINB, pin - 8);
    }
  }
  
  static inline void setHigh() __attribute__((always_inline)) {
    if (pin < 8) {
      PORTD |= mask;
    } else {
      PORTB |= mask;
    }
  }
  
  static inline void setLow() __attribute__((always_inline)) {
    if (pin < 8) {
      PORTD &= ~mask;
    } else {
      PORTB &= ~mask;
    }
  }

  // If b happens to be known at compilation time, then the if statement here will be optimized as well.
  static inline void write(bool b) __attribute__((always_inline)) {
    if (b) {
      setHigh();
    } else {
      setLow();
    }
  }

#else
  
#error Your MCU is not supported by FastPin class. Please add it!
    
#endif // MCU defines
	
/** 
 * This is to make a bunch of different pins appear as an 8-bit bus. 
 */
template<
  typename pinD0, typename pinD1, typename pinD2, typename pinD3, 
  typename pinD4, typename pinD5, typename pinD6, typename pinD7
>
class PinBus {
  
public:

  static void setOutput() {
     pinD0::setOutput();
     pinD1::setOutput();
     pinD2::setOutput();
     pinD3::setOutput();
     pinD4::setOutput();
     pinD5::setOutput();
     pinD6::setOutput();
     pinD7::setOutput();
  }

  static void setInput(bool pullup = false) {
     pinD0::setInput(pullup);
     pinD1::setInput(pullup);
     pinD2::setInput(pullup);
     pinD3::setInput(pullup);
     pinD4::setInput(pullup);
     pinD5::setInput(pullup);
     pinD6::setInput(pullup);
     pinD7::setInput(pullup);
  }

  static void write(uint8_t b) {
    pinD0::write(b & (1 << 0));
    pinD1::write(b & (1 << 1));
    pinD2::write(b & (1 << 2));
    pinD3::write(b & (1 << 3));
    pinD4::write(b & (1 << 4));
    pinD5::write(b & (1 << 5));
    pinD6::write(b & (1 << 6));
    pinD7::write(b & (1 << 7));
  }

  static uint8_t read() {
    return (pinD0::read() << 0)
      | (pinD1::read() << 1)
      | (pinD2::read() << 2)
      | (pinD3::read() << 3)
      | (pinD4::read() << 4)
      | (pinD5::read() << 5)
      | (pinD6::read() << 6)
      | (pinD7::read() << 7);
  }
};
  
// Cannot seem to pop the options correctly, have to reset
#pragma GCC reset_options

}; // FastPin class

} // namespace
