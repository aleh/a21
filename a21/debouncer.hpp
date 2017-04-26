//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>

namespace a21 {

/** 
 * Simple debouncer logic for Arduino.
 */
template<class T, int timeout = 10, bool initial_value = false>
class Debouncer {

private:

  // The debounced value.
  volatile bool _value;

  // True, if the new value along with its timestamp is known but is held till timeout elapses.
  volatile bool _holding;

  // The new value being held.
  volatile bool _heldValue;

  // When the new value was read.
  volatile unsigned long _timestamp;
  
public:

  Debouncer() 
    : _value(initial_value), _holding(false) {}

  /** Debounced value. */
  bool value() const {
    return _value;
  }

  /** Called when the debounced value has changed. Interrupts are enabled. */
  void valueDidChange() { 
  }

  /** Should be called every time a new value is read, for example from a pin interrupt handler. */
  void setValue(bool value) {
    _holding = true;
    _heldValue = value;
    _timestamp = millis();
  }

  /** 
   * Should be called periodically (from the main loop) to check if the new value being held has finally settled. 
   * Note that it assumes that interrupts are enabled.
   */
  void check() {
    
    noInterrupts();

    if (_holding && (int)(millis() - _timestamp) >= timeout) { 
      
      // OK, enough time has passed, let's proclaim the value being held as a debounced one.
      _holding = false;
      _value = _heldValue;
      
      interrupts();
      
      static_cast<T*>(this)->valueDidChange();
      
    } else {
      interrupts();
    }
  }
};

} // namespace
