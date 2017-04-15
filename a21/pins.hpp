//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#include <arduino.h>

namespace a21 {

/** 
 * Pin for the cases when a template class requires one, but you don't really need it, like if you've left 
 * a secondary pin unconnected, etc. 
 */
class UnusedPin {
public:

  static inline void setOutput() {}
  static inline void setInput(bool) { }
  
  static inline bool isHigh() { return false; }
  
  static inline void setHigh() {}
  static inline void setLow() {}
  
  static inline void write(bool) { }
};

/** 
 * Wrapper for a pin that uses standard Arduino library calls. 
 */
template<int pin>
class SlowPin {
public:

  static inline void setOutput() {
    pinMode(pin, OUTPUT);
  }
  
  static inline void setInput(bool pullup) {
    pinMode(pin, pullup ? INPUT_PULLUP : INPUT);
  }
  
  static inline bool isHigh() {
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
 * Wrapper for a pin that uses single instruction access to the corresponding port. Assuming standard Arduino Uno 
 * pin -> port mapping here. 
 */
template<int pin>
class FastPin {

private:

  // Calculating our bitmask in one place. Unfortunately cannot make this trick for the port registers it seems.
  static uint8_t const mask = (pin < 8) ? _BV(pin) : _BV(pin - 8);
  
public:

#pragma GCC optimize ("O2")
	
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
  
  static inline bool isHigh() __attribute__((always_inline)) {
    if (pin < 8) {
      return bit_is_set(PORTD, pin);
    } else {
      return bit_is_set(PORTD, pin - 8);
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
  
#pragma GCC reset_options
};

} // namespace
