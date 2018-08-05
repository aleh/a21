//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

#include <arduino.h>

namespace a21 {

/** To represent EC-11 encoder rotation events, such as "clock-wise rotation by 5 steps". */
struct EC11Event {

	enum Type {

    /** No event yet. */
		None = 0,
    
    /** Clock-wise rotation. */
		StepCW, 
    
    /** Counter-clockwise rotation. */
		StepCCW 
	};

	/** Direction of the rotation. */
	Type type;

	/** Number of steps in the current direction. */
	uint8_t count;
	
	EC11Event() : type(None), count(0) {}
};

/**
 * Helps reading EC-11 rotary encoders. 
 * Does not depend on pin numbers, so can be fed from an interrupt handler or from a polling loop.
 * A typical setup involves reading two digital input pins with internal pull-ups either from a pin change 
 * interrupt handler or periodically and then passing their states to checkPins().
 */
class EC11 {
	
private:

	// Here we store last 4 pin states, as pairs of bits, with the least significant ones 
	// representing the most recent state.
	uint8_t _lastPinStates;
	
	// The latest not yet read event.
	EC11Event _event;
  	
	/** Called internally every time we see a quad of pin states representing a single defined step. */
	void addEvent(EC11Event::Type type) {
		
		if (_event.type == type) {
			// Similar event, consolidate with the recent one by incrementing the counter, 
      // saturating it, if needed, to avoid an overflow.
			if (_event.count != 0xFF) {
				_event.count++;
			}
		} else {
			// New direction, restart the step counter.
			_event.type = type;
			_event.count = 1;
		}
	}
  				
public:

	EC11() : _lastPinStates(0) {}
  
  /** Resets the current sequence of events, if there is any. Handy when we know that the most recent events could not be caused by normalpin transitions. */
  void reset() {
    _lastPinStates = 0;
  }

	/** 
	 * Must be called either from an interrupt handler associated with pin changes, 
	 * or periodically from a polling loop. 
	 * The current states of both pins should be passed here. 
	 * Note that we expect that the default state of both pins is HIGH when the encoder is at a stable point.
	 */
	void checkPins(bool pinAState, bool pinBState, bool switchPinState = true) {
		
		uint8_t state = (pinBState << 1) | pinAState;
		if (state != (_lastPinStates & 0x3)) {
		
			// OK, the state of pins has changed compared to the last known, let's record it.
			
			_lastPinStates = (_lastPinStates << 2) | state;
		
			// If we see a sequence of codes corresponding to a single step, then let's record an event.
			if (_lastPinStates == 0x87) {
				addEvent(EC11Event::StepCCW);
			} else if (_lastPinStates == 0x4B) {
				addEvent(EC11Event::StepCW);
			}
		}
	}
  
	/** 
	 * This should be called from a normal event loop periodically to check if there are new events coming 
	 * from the encoder. When true is returned, then the EC11Event structure the `e` parameter references 
   * is filled with the most recent event (which is then marked as "read" and won't be returned the next time).
	 */
	bool read(EC11Event *e) {

		// checkPins() above might be called from an interrupt handler, so we need to make sure we won't access the event at the same time.
		noInterrupts();
		
		if (_event.count == 0) {
			// Well, no events yet.
			interrupts();
			return false;
		}

		// OK, got one, let's copy it.
		*e = _event;
		
		// Let's reset the counter, so only new events will be seen the next time.
		_event.count = 0;
		
		// We assume this function is always called from a normal interrupt-enabled state, so simply re-enabling them.
		interrupts();
	
		return true;
	}  
};

/** To represent EC11 switch events. */
enum EC11PressEvent {
  EC11PressEventNone,
  EC11PressEventDown,
  EC11PressEventUp
};

/** 
 * Allows to use a EC11 with a single pin. Handy when you are out of pins but can afford to do ADC fequently enough.
 *
 * The setup uses 3 resistors here: all are connected to your ADC pin with their one end and to Vcc, pin A and pin B 
 * of your encoder with their other ends. Their resistances are passed here as R, RA, and RB correspondingly.
 * The max resolution of your ADC is passed here as well (V00). Note that RA should be greater than RB. 
 * The default values are optimized for low current draw and maximum difference in ADC values.
 *
 * Working with this class is very similar to working with EC11: you do ADC on your pin fast enough 
 * and call checkValue() instead of checkPins(). You can then call read() less freqiently from your interrupt-enabled 
 * loop to check if any event was recorded.
 */
template<
  const uint32_t R = 20000, 
  const uint32_t RA = 68000, 
  const uint32_t RB = 47000, 
  const uint16_t V00 = 0x3FF
>
class OnePinEC11 : protected EC11 {
  
  typedef OnePinEC11<R, RA, RB, V00> Self;
  
  static Self& getSelf() {
    static Self s = Self();
    return s;
  }
  
  static const uint32_t RAB = (RA * RB) / (RA + RB);
  
  static const uint16_t V10 = V00 * RA / (R + RA);     
  static const uint16_t V01 = V00 * RB / (R + RB);
  static const uint16_t V11 = V00 * RAB / (R + RAB);
  
  bool _lastSwitchState;
  
  /** The next unread press event. */
  EC11PressEvent _pressEvent;
    
  void checkSwitchPin(bool switchPinState) {
      
    if (_lastSwitchState != switchPinState) {
      
      _lastSwitchState = switchPinState;
      
      _pressEvent = _lastSwitchState ? EC11PressEventUp : EC11PressEventDown;
      
      // Disregard the recent chain of transitions, if any, because the switching could cause some fake transitions.
      EC11::reset(); 
    }
  }
      
public:
  
  /** Reads (and "eats") the next encoder switch state change event. */
  EC11PressEvent readPress() {
    
  		noInterrupts();
      
      // Read and "eat" the event, so it's not returned the next time.
      EC11PressEvent result = _pressEvent;      
      _pressEvent = EC11PressEventNone;
      
      interrupts();

      return result;
  }

  /** Reads (and "eats") the next encoder rotation event. */
  bool readRotation(EC11Event *e) {
    return EC11::read(e);
  }

  /** Should be called often enough from your ADC conversion code. */
  void checkValue(uint16_t v) {
    
    if (v >= V11 / 2) {
    
      if (v >= (V00 + V10) / 2) {
        EC11::checkPins(true, true);
      } else if (v >= (V10 + V01) / 2) {
        EC11::checkPins(false, true);
      } else if (v >= (V01 + V11) / 2) {
        EC11::checkPins(true, false);
      } else {
        EC11::checkPins(false, false);
      }
      
      checkSwitchPin(true);
      
    } else {
      
      checkSwitchPin(false);
    }
  }
};

} // namespace
