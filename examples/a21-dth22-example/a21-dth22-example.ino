//
// a21 — Arduino Toolkit. Example for DHT22 class.
// Copyright (C) 2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#include <a21.hpp>

using namespace a21;

// The first parameter is the pin the sensor is attached to. 
// The second one is true in case a built-in pull-up should be activated.
DHT22< FastPin<2>, false > dht22;

// Here we are showing sensor readings on a Nokia display, though anything else would work of course.
// The pins are in the order of the display: RST, CE, DC, DIN, CLK. 
// I am not using RST and CE here, so it can easily work with Digispark.
typedef PCD8544< UnusedPin, UnusedPin, FastPin<5>, FastPin<4>, FastPin<0> > LCD;
LCD lcd;

// A simple text console that is able to render itself to the LCD.
PCD8544Console<LCD, PCD8544FontPixelstadTweaked> console;

void setup() {
  
  lcd.begin();  
  
  // The console is clean initially, so drawing it effectively clears the display.
  console.draw();
}

void loop() {
    
  console.clear();
  console.println(F("a21 - DHT22 example"));
  console.println();

  // Both values will be premultiplied by 10. I don't divide them by 10 in the DHT22 to avoid using floating point. 
  // Also, some monitoring applications can always work with premultiplied values.
  int16_t temperature;
  uint16_t humidity;
  if (dht22.read(temperature, humidity)) {
    
    console.print(F("Temperature: "));
    console.print(temperature / 10);
    console.println(F("C"));
    
    console.print(F("Humidity: "));
    console.print(humidity / 10);
    console.println(F("%"));
    
  } else {
    
    console.print(F("Cannot read DHT22"));
  }

  console.draw();

  delay(1000);
}

