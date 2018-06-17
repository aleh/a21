//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <Arduino.h>

#include "flashstring.hpp"

namespace a21 {
  
/** 
 * Adds an overloaded print/println() functions for the given class, which needs to provide write(char ch) and lf(void) methods. 
 */
template<typename T>
class Print {
  
public:
  
	static void lf() {
		T::write('\n');
	}

	static void print(char ch) {
		T::write(ch);
	}

	static void print(const char *str) {
		const char *src = str;
		char ch;
		while (ch = *src++) {
			T::write(ch);
		}
	}

	static void print(FlashStringPtr str) {
		const char *src = (const char *)str;
		char ch;
		while (ch = pgm_read_byte(src++)) {
			T::write(ch);
		}
	}

	static void print(int n) {
		char buf[5 + 2];
		itoa(n, buf, 10);
		print(buf);
	}

	static void print(unsigned int n) {
		char buf[5 + 1];
		utoa(n, buf, 10);
		print(buf);
	}

	static void print(long n) {
		char buf[10 + 2];
		ltoa(n, buf, 10);
		print(buf);
	}

	static void print(unsigned long n) {
		char buf[10 + 1];
		ltoa(n, buf, 10);
		print(buf);
	}

	static void println(const char *str) {
		print(str);
		T::lf();
	}

	static void println(FlashStringPtr str) {
		print(str);
		T::lf();
	}  

	static void println(int n) {
		print(n);
		T::lf();
	}

	static void println(unsigned int n) {
		print(n);
		T::lf();
	}

	static void println(long n) {
		print(n);
		T::lf();
	}  

	static void println(unsigned long n) {
		print(n);
		T::lf();
	}  

	static void println() {
		T::lf();
	}
};

} // namespace a21
