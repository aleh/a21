//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>

namespace a21 {

/** 
 * A pin that is not connected anywhere.
 * This is handy when a template requires a pin, but it is actually optional, i.e can be left unconnected 
 * or is driven by other circuit or user's code.
 */
template<bool value = false>
class UnusedPin {
public:
	
	/** Can be handy for the user classes to know if the pin is actually unused, so any related actions can be skipped as well. */
	static const bool unused = true;

  static inline void setOutput() {}
  static inline void setInput(bool) { }
  
  static inline bool read() { return value; }
  
  static inline void setHigh() {}
  static inline void setLow() {}
  
  static inline void write(bool) { }
};

/** This is to invert the levels of a pin without changing the code using it. */
template<typename Pin>
class InvertedPin {
public:
	
	static const bool unused = false;

  static inline void setOutput() { Pin::setOutput(); }
  static inline void setInput(bool pullup) { Pin::setInput(pullup); }
  
  static inline bool read() { return !Pin::read(); }
  
  static inline void setHigh() { Pin::setLow(); }
  static inline void setLow() { Pin::setHigh(); }
  
  static inline void write(bool value) { Pin::write(!value); }
};

/** 
 * A pin that uses standard Arduino library calls (digitalRead() and friends).
 * This is handy when debugging FastPin or with boards that are not supported by it.
 */
template<int pin>
class SlowPin {
public:
  
  static const int Pin = pin;
	
	static const bool unused = false;

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
    
#pragma GCC optimize ("O2")
		
private:

	typedef volatile uint8_t *port_ptr;
	
	// The MCU-specific parts below should provide the following definitions for the code that follows:
	// static constexpr uint8_t mask; // A mask corresponding to our pin in the port registers.
	// static port_ptr port(); // The corresponding PORT* register.
	// static port_ptr ddr(); // DDR* register.
	// static port_ptr in(); // PIN* register.
	
	// Note that it's assumed that the same mask is used for all registers, which is true only for currently 
	// supported boards.	
	
	// Also note that I was using constexpr before instead of inline functions but later versions of GCC 
	// stopped supporting constexpr with numbers reinterpreted as pointers, something that all port addresses are.
	// Shame, this was useful and the restriction makes no sense for MCU-specific code we are dealing with,
	// see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=49171#c18.

// ATTiny85-based boards, like Digispark.
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)

	// Pins 0-7 -> PORTB0:8  
	static constexpr uint8_t const mask = _BV(pin);
	static port_ptr port() __attribute__((always_inline)) { return &PORTB; }
	static port_ptr ddr() __attribute__((always_inline)) { return &DDRB; }
	static port_ptr in() __attribute__((always_inline)) { return &PINB; }
  
// ATmega328-based boards, like Arduino Uno or Nano.
#elif defined(__AVR_ATmega328P__)
	
	// Pins 0-7 -> PORTD0:7
	// Pins 8-13 -> PORTB0:5 (a crystal is connected to pins 6 and 7)
	// Pins A0-A7 -> PORTC

	static constexpr uint8_t const mask = _BV(pin <= 7 ? pin : ((pin < A0) ? pin - 8 : pin - A0));
	
	static port_ptr regForPin(port_ptr D, port_ptr B, port_ptr C) __attribute__((always_inline)) {
		return pin <= 7 ? D : ((pin < A0) ? B : C);
	}

	static port_ptr port() __attribute__((always_inline)) { return regForPin(&PORTD, &PORTB, &PORTC); }
	static port_ptr ddr() __attribute__((always_inline)) { return regForPin(&DDRD, &DDRB, &DDRC); }
	static port_ptr in() __attribute__((always_inline)) { return regForPin(&PIND, &PINB, &PINC); }

// ATmega32u4-based boards, like Arduino Leonardo and Micro.
#elif defined(__AVR_ATmega32U4__)
	
	// It's not as nice for these Arduinos, still the mapping will be performed at compile time.
		
	static constexpr uint8_t maskForPin() {
		return
			(pin == 3 || pin == 17 || pin == 23) ? _BV(0) : (
				(pin == 2 || pin == 15 || pin == 22) ? _BV(1) : (
					(pin == 0 || pin == 16) ? _BV(2) : (
						(pin == 1 || pin == 14) ? _BV(3) : (
							(pin == 4 || pin == 8 || pin == 21 || pin == 24 || pin == 26) ? _BV(4) : (
								(pin == 9	|| pin == 20 || pin == 27 || pin == 30) ? _BV(5) : (
									(pin == 5 || pin == 7 || pin == 10 || pin == 12 || pin == 19 || pin == 28 || pin == 29) ? _BV(6) : (
										(pin == 6 || pin == 11 || pin == 13 || pin == 18 || pin == 25) ? _BV(7) : 0
									)
								)
							)
						)
					)
				)
			);
	}
	
	static constexpr uint8_t mask = maskForPin();
	
	static port_ptr regForPin(port_ptr B, port_ptr C, port_ptr D, port_ptr E, port_ptr F) __attribute__((always_inline)) {
		return 
			(pin == 8 || pin == 9 || pin == 10 || pin == 11 || pin == 14 || pin == 15 || pin == 16 || pin == 17 || pin == 26 || pin == 27 || pin == 28) ? B : (
				(pin == 5 || pin == 13) ? C : (
					(pin == 0 || pin == 1 || pin == 2 || pin == 3 || pin == 4 || pin == 6 || pin == 12 || pin == 24 || pin == 25 || pin == 29 || pin == 30) ? D : (
						(pin == 7) ? E : (
							(pin == 18 || pin == 19 || pin == 20 || pin == 21 || pin == 22 || pin == 23) ? F : 0
						)
					)
				)
			);
	}
	
	static port_ptr port() __attribute__((always_inline)) { return regForPin(&PORTB, &PORTC, &PORTD, &PORTE, &PORTF); }
	static port_ptr ddr() __attribute__((always_inline)) { return regForPin(&DDRB, &DDRC, &DDRD, &DDRE, &DDRF); }
	static port_ptr in() __attribute__((always_inline)) { return regForPin(&PINB, &PINC, &PIND, &PINE, &PINF); }
	
#else
#error Your MCU is not supported by FastPin yet.    
#endif // MCU defines
	
public:

  static const int Pin = pin;  
	
	static const bool unused = false;
	
  static inline void setOutput() __attribute__((always_inline)) {
		*ddr() |= mask;
  }
  
  static inline void setInput(bool pullup) __attribute__((always_inline)) {
		*ddr() &= ~mask;
    if (pullup) {
      *port() |= mask;
    } else {
      *port() &= ~mask;
    }
  }
  
  static inline bool read() __attribute__((always_inline)) {
		return (*in()) & mask;
  }
  
  static inline void setHigh() __attribute__((always_inline)) {
		*port() |= mask;
  }
  
  static inline void setLow() __attribute__((always_inline)) {
		*port() &= ~mask;
  }

  // If b happens to be known at compile time, then the if statement here will be optimized as well.
  static inline void write(bool b) __attribute__((always_inline)) {
    if (b) {
      setHigh();
    } else {
      setLow();
    }
  }
  
// Cannot seem to pop the options correctly, have to reset
#pragma GCC reset_options

}; // FastPin class
	
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

} // namespace
