//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

namespace a21 {

/** 
 * Wrapper for a PCD8544 LCD display (such as the one that was used on Nokia 5110).
 * Note that the 'slow' parameter should be true only if this happen to be ported on a faster MCU 
 * and we need to ensure clock speed is less than 4 MHz. Clock speed will be less than 2 MHz on 16MHz Arduinos.
 */
template<typename pinRST, typename pinCE, typename pinDC, typename pinDIN, typename pinCLK, bool slow = false>
class PCD8544 {
	
  /** Delays execution for at least a half of the minimal clock period, which is 1/4 us in the datasheet. */
  static inline void delay125() __attribute__((always_inline)) {
	  _delay_us((1.0 / 4000000) / 2);
  }

  static inline void writeBit(bool b) __attribute__((always_inline)) {
    pinDIN::write(b);
    pinCLK::setLow();
    if (slow)
      delay125();
    pinCLK::setHigh();
    if (slow)
      delay125();
  }  

  enum ValueType {
    Command,
    Data
  };
  
  static void write(ValueType valueType, uint8_t value) {

    pinDC::write(valueType == Data);

    writeBit(value & _BV(7));
    writeBit(value & _BV(6));
    writeBit(value & _BV(5));
    writeBit(value & _BV(4));
    writeBit(value & _BV(3));
    writeBit(value & _BV(2));
    writeBit(value & _BV(1));
    writeBit(value & _BV(0));
  }  

  static inline void beginWriting() {
    pinCLK::setLow();
    pinCE::setLow();
  }
  
  static inline void endWriting() {
    pinCE::setHigh();
    pinCLK::setLow();
    pinDIN::setLow();	
  }
        
  enum FunctionSetCommand {
    FunctionSet = 0x20,
    /** 0 - basic command set, 1 - extended command set. */
    H = 1,
    /** 0 - horizontal addressing mode, 1 - vertical addressing mode. */
    V = 2,
    /** 0 - chip active, 1 - chip in power down mode. */
    PD = 4
  };

  enum DisplayControlCommand {
    DisplayControl = 0x08,
    D = 0x04,
    E = 0x01,
    DisplayBlank = ~(D | E),
    NormalMode = D & ~E,
    AllSegmentsOn = ~D & E,
    InverseVideoMode = D | E
  };

  enum SetXAddressCommand {
    SetXAddress = 0x80,
    SetXAddressMask = 0x7F
  };
  
  enum SetYAddressCommand {
    SetYAddress = 0x40,
    SetYAddressMask = 0x07
  };

  enum TemperatureControlExtendedCommand {
    TemperatureControl = 0x04,
    TemperatureControlMask = 0x3
  };

  enum BiasSystemExtendedCommand {
    BiasSystem = 0x10,
    BiasSystemMask = 0x07
  };

  enum SetVopExtendedCommand {
    SetVop = 0x80,
    SetVopMask = 0x7F
  };

  static inline void extendedCommandSet(bool extended) {
    write(Command, extended ? (FunctionSet | H) & ~(PD | V) : (FunctionSet | 0) & ~(PD | V | H));
  }
  
  static inline void setAddressInternal(uint8_t col, uint8_t row) {
    extendedCommandSet(false);
    write(Command, SetXAddress | col);
    write(Command, SetYAddress | row);
  }
    
public:

  PCD8544() {
  }
  
  /** Number of rows (in the sense of addressing), every row corresponds to 8 horizontal lines of actual pixels. */
  static const int Rows = 6;
  
  /** Number of columns (again, in the sense of addressing), though unlike rows every column corresponds to 1 vertical line of pixels. */
  static const int Cols = 84;
  
  static void operatingVoltage(uint8_t value) {
    beginWriting();
    extendedCommandSet(true);
    write(Command, SetVop | value);
    endWriting();
  }

  static void setAddress(uint8_t col, uint8_t row) {
    beginWriting();
    setAddressInternal(col, row);
    endWriting();
  }
  
  static void clear() {
    beginWriting();
    setAddressInternal(0, 0);
    for (int i = 0; i < Rows * Cols; i++) {
      write(Data, 0);
    }
    endWriting();
  }
  
  static void config(uint8_t operatingVoltage, uint8_t biasSystem, uint8_t temperatureControl) {
    
    beginWriting();

    // TODO: perhaps make the display mode a parameter?
    extendedCommandSet(false);
    write(Command, DisplayControl | NormalMode);
        
    extendedCommandSet(true);    
    write(Command, SetVop | (operatingVoltage & SetVopMask));
    write(Command, BiasSystem | (biasSystem & BiasSystemMask));
    write(Command, TemperatureControl | (temperatureControl & TemperatureControlMask));
            
    endWriting();
  }
  
  /** Initializes the device and transport pins. */
  static void begin(uint8_t operatingVoltage, uint8_t biasSystem = 4, uint8_t temperatureControl = 0) {
    
    pinDIN::setOutput();
    pinDIN::setLow();
    
    pinCLK::setOutput();
    pinCLK::setLow();
    
    pinDC::setOutput();
    pinDC::setLow();
    
    pinRST::setOutput();    
    
    pinCE::setOutput();
    pinCE::setHigh();

    pinRST::setLow();
    delay125();
    pinRST::setHigh();
    delay125();

    config(operatingVoltage, biasSystem, temperatureControl);
    
    clear();
  }
  
  /** Transports a bunch of bytes for the given row starting with the byte representin the given column. */
  static void writeRow(uint8_t col, uint8_t row, const uint8_t *data, uint8_t data_length) {
    beginWriting();
    setAddressInternal(col, row);
    const uint8_t *src = data;
    for (uint8_t c = data_length; c > 0; c--) {
      uint8_t b = *src++;
      write(Data, b);
    }
    endWriting();
  }
};

} // namespace
