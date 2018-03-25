//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>

#include "clock.hpp"

namespace a21 {

/** 
 * Reads DHT22 and compatible temperature/humidity sensors connected to the given pin.
 * Should have pretty compact code (~500 bytes) because it does not use floating point even to divide by 10 here.
 * The `pin` parameter should be FastPin-compatible.
 * The `pullup` parameter is true in case an internal pullup on the pin should be enabled.
 */
template<typename pin, bool pullup, typename clock = ArduinoClock>
class DHT22 {
  
private:
  
  static uint8_t wait_while_pin(bool value, uint8_t min_timeout, uint8_t max_timeout) {
    
    uint8_t start = clock::micros8();
    uint8_t time;
    bool late;
    do {
      time = clock::micros8() - start;
      late = time > max_timeout;
    } while (pin::read() == value && !late);
    
    if (late || time < min_timeout)
      return 0;
    else if (time <= 0)
      return 1;
    else
      return time;
  }
    
public:
  
  /** Returns false in case the reading was not successful. */
  static bool read(int16_t& temperature, uint16_t& humidity) {

    // First we pull the line low for at least 1 ms
    pin::setOutput();
    pin::setLow();
    clock::delay(1);
    
    bool result = false;
    uint8_t response[5];
    
    noInterrupts();
    
    do {
      
      // Then release it and wait for the sensor to pull the line low. 
      // It should do so within 20-40 us, but we give it a bit more time.
      pin::setInput(pullup);
      
      clock::delayMicroseconds(1);
      
      if (!wait_while_pin(true, 1, 60))
        break;
    
      // Now the sensor should pull the line low for about 80 us.
      if (!wait_while_pin(false, 1, 100))
        break;
      
      // And then back high for about 80 us as well.
      if (!wait_while_pin(true, 1, 100))
        break;
            
      //
      // Now the bits of the response should follow: 50 us low to begin a bit, then 26-28us for 0 and about 70 us for 1.
      // We need 40 bits total.
      //
      for (uint8_t i = 0; i < sizeof(response); i++) {
      
        uint8_t m = 0;
        for (uint8_t b = 8; b > 0; b--) {
          
          if (!wait_while_pin(false, 1, 70))
            goto exit;          
          
          uint8_t time = wait_while_pin(true, 1, 100);
          if (time == 0)
            goto exit;
          
          if (time > 48)
            m |= 1;
          m <<= 1;
        }
                
        response[i] = m;
      }
      
      uint8_t checksum = response[0];
      checksum += response[1]; 
      checksum += response[2]; 
      checksum += response[3]; 
      if (checksum == response[4]) {
        temperature = (((uint16_t)(response[2] & 0x7F) << 8) | response[3]);
         if (response[2] & 0x80)
          temperature = -temperature;
        humidity = (((uint16_t)response[0] << 8) | response[1]);
        result = true;
      }
      
    } while (0);
    
  exit:
    interrupts();
    
    return result;
  }
};
  
} // namepsace
