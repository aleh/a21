//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include "Arduino.h"

namespace a21 {
  
/** 
 * Digispark boards have no EEPROM library, so here is a simple one.
 * TODO: something is wrong here, investigate
 */
class EEPROM {
public:
  
  static uint8_t read(uint16_t address) {
    // Wait for any previous write to complete.
    while (EECR & _BV(EEPE))
      ;
    EEARL = address;
    EECR |= _BV(EERE);
    return EEDR;
  }
  
  static void update(uint16_t address, uint8_t data) {

    if (read(address) == data)
      return;
    
    // No need to wait for the prvious write to complete, already did this in read().
        
    // Make sure we'll use Erase & Write in one operation.
    EECR &= ~(_BV(EEPM1) | _BV(EEPM0));
    
    EEARL = address;
    EEDR = data;
    
    // These two bits have to be set separately: first Master Program Enable, then Program Enable.
    EECR |= _BV(EEMPE);
    EECR |= _BV(EEPE);
  }
};

} // namespace a21
