//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

namespace a21 {

// Regular Arduinos and Digispark boards have different definition for the progmem strings,
// thus using an alias here for a common ground.
#if defined(ARDUINO_AVR_DIGISPARK)
		typedef fstr_t *FlashStringPtr;
#else 
		// Assuming a regular Arduino.
		typedef const __FlashStringHelper *FlashStringPtr;
#endif

};
