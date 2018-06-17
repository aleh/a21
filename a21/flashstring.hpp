//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2017, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

namespace a21 {

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
typedef fstr_t *FlashStringPtr;
#else
typedef const __FlashStringHelper *FlashStringPtr;
#endif

};
