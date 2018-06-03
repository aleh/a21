//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#include <Arduino.h>

namespace a21 {

/** 
 * Basic software I2C. 
 * If builtInPullups is true, then the built-in pull-ups will be used with SCL and SDA pins.
 */
template<
  typename pinSCL, 
  typename pinSDA, 
  bool builtInPullups = true,
  uint32_t frequency = 400000L,
  typename Clock = ArduinoClock
>
class SoftwareI2C {

private:
  
  static inline void ReleaseSCL() {
    pinSCL::setInput(builtInPullups);
  }

  static inline void PullDownSCL() {
    pinSCL::setLow();
    pinSCL::setOutput();
  }  
  
  static inline bool IsSCLHigh() {
    return pinSCL::read();
  }  

  static inline void ReleaseSDA() {
    pinSDA::setInput(builtInPullups);
  }

  static inline void PullDownSDA() {
    pinSDA::setLow();
    pinSDA::setOutput();
  }
  
  static inline bool IsSDAHigh() {
    return pinSDA::read();
  }
  
  static inline void delay(uint8_t t) __attribute__((always_inline)) {
    Clock::delayMicroseconds(t * 1000000L / frequency);
  }
  
public:
  
  static void begin() {
    ReleaseSCL();
    ReleaseSDA();
  }  
  
  static bool startWriting(uint8_t slave_address) {
    
    // Assuming SCL is released, so pulling down SDA to indicate the start condition.
    PullDownSDA();
    delay(1);
    
    return write(slave_address << 1);
  }
  
  static bool write(uint8_t b) {
    
    for (uint8_t bit = 8; bit != 0; bit--, b <<= 1) {
      
      PullDownSCL();
      
      delay(0.1);
      
      if (b & 0x80) {
        ReleaseSDA();
      } else {
        PullDownSDA();
      }
      
      delay(0.4);
              
      ReleaseSCL();
      
      delay(0.5);
      
      // We don't support clock stretching.
      if (!IsSCLHigh())
        return false;
    }
    
    // Acknowledge bit.
    PullDownSCL();
    ReleaseSDA();
    delay(0.5);
    ReleaseSCL();
    delay(0.5);
    
    return !IsSDAHigh();
  }
  
  static inline void stop() {
    PullDownSCL();
    delay(0.5);
    PullDownSDA();
    ReleaseSCL();
    delay(1);
    ReleaseSDA();    
  }
  
  static bool write(const uint8_t *data, uint8_t data_length) {
    for (uint8_t i = 0; i < data_length; i++) {
      if (!write(data[i]))
        return false;
    }
    return true;
  }
      
  static bool write(uint8_t slave_address, const uint8_t *data, uint8_t data_length) {
    
    bool result = startWriting(slave_address) && write(data, data_length);
        
    stop();
    
    return result;
  }
  
};  
  
};