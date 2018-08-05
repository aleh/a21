//
// a21 — Arduino Toolkit.
// Copyright (C) 2016-2018, Aleh Dzenisiuk. http://github.com/aleh/a21
//

#pragma once

namespace a21 {

/** 
 * Simple MIDI stream parser. 
 * Inherit, call handleByte() for every byte of your incoming stream and override handleEvent() to handle the messages.
 */
template <class T>
class MIDIParser {

public:

  /** MIDI events we recognize. 
	 * (Note that the values are status bytes masked with 0x70, not the actual event values.) */
  enum Event : uint8_t {
		
		/** Args: note, velocity (0-127). */
    EventNoteOff = 0x00,
		
		/** Args: note, velocity (0-127). */
    EventNoteOn = 0x10,
		
		/** Args: note, velocity (0-127). */
    EventPolyAftertouch = 0x20,
		
		/** Args: control number (0-119, 120-127 are reserved), value (0-127). */
    EventControlChange = 0x30,
		
		/** Args: program number (0-127). */
    EventProgramChange = 0x40,
		
		/** Args: velocity (0-127). */
    EventAftertouch = 0x50,
		
		/** Args: most significant 7 bits of the value, then least significant 7 bits of the value. 
		 * 0x2000 means the wheel is in the middle. */
    EventPitchBend = 0x60,
		
    /** To represent the events we don't parse. */
    EventUnknown = 0xF0

  } event;

  /** A note without an octave. You can find it as a remainder after division of a MIDI note by 12. */
  enum Note : uint8_t {
    NoteC = 0,
    NoteCs,
    NoteD,
    NoteDs,
    NoteE,
    NoteF,
    NoteFs,
    NoteG,
    NoteGs,
    NoteA,
    NoteAs,
    NoteB
  };
	
private:
	
	inline T& getSelf() {
		return static_cast<T&>(*this);
	}

  /** The number of arguments (parameters) expected for the given MIDI event. Note that we don't handle everything. */
  static uint8_t argsForEvent(Event e) {
    return (EventNoteOff <= e && e <= EventControlChange || e == EventPitchBend) ? 2 : 1;
  }

  // The channel of the current MIDI event.
  uint8_t channel;

  // The arguments of the current MIDI event. 
  uint8_t args[2];

  // Number of valid arguments in args collected so far.
  uint8_t argsCollected;

	/** Calls the handler method if we've got enough args for the current MIDI event. */
	void handleEventIfFinished() {
    if (event != EventUnknown && argsCollected == argsForEvent(event)) {
				getSelf().handleEvent(event, channel, args);
        event = EventUnknown;
    }    
  }
  
public:

  void begin() {
    event = EventUnknown;
  }
  
  void handleByte(uint8_t b) {

    if (b & 0x80) {

      // A status byte always begins a new message.
      
      if (event != EventUnknown) {
        // Another command started before the previous one was fully read.
        // Something is wrong with our expectations or the stream is corrupted.
        // Can output some diagnostics or blink lights if it's important.
      }
      
      event = (Event)(b & 0x70);
      channel = b & 0x0F;

      // We simply skip those extra events.
      if (event >= EventPitchBend) {
        event = EventUnknown;
      }
      
      argsCollected = 0;
      
      // We don't currently have events without args, but still let's call to not forget later.
      handleEventIfFinished();
      
    } else {
      
      // Data bytes aka event arguments.
      
      if (event == EventUnknown) {
        // Skipping stray bytes or args of unknown events.
      } else {
        // Collecting event bytes.
        args[argsCollected++] = b;      
        handleEventIfFinished();
      }
    }
  }
	
  /** 
	 * This is eventually called by the parser for every incoming event. The default implementation calls 
	 * a corresponding handle*() function below sort of preparsing the parameters.
	 */
  void handleEvent(Event event, uint8_t channel, const uint8_t *args) {
		
		switch (event) {
			
			case EventNoteOff:
				getSelf().handleNoteOff(channel, args[0], args[1]);
				break;
				
			case EventNoteOn:
				getSelf().handleNoteOn(channel, args[0], args[1]);
				break;
				
			case EventPolyAftertouch:
				getSelf().handlePolyAftertouch(channel, args[0], args[1]);
				break;
				
			case EventControlChange:
				getSelf().handleControlChange(channel, args[0], args[1]);
				break;
			
			case EventProgramChange:
				getSelf().handleProgramChange(channel, args[0]);
				break;
				
			case EventAftertouch:
				getSelf().handleAftertouch(channel, args[0]);
				break;
				
			case EventPitchBend:
				getSelf().handlePitchBend(channel, ((uint16_t)args[0] << 7) | args[1]);
				break;
			
			case EventUnknown:
				break;
		}
  }	
	
	void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
	}
	
	void handleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
	}
	
	void handlePolyAftertouch(uint8_t channel, uint8_t note, uint8_t velocity) {
	}
	
	void handleControlChange(uint8_t channel, uint8_t control, uint8_t value) {
	}
	
	void handleProgramChange(uint8_t channel, uint8_t program) {
	}
	
	void handleAftertouch(uint8_t channel, uint8_t value) {
	}
	
	void handlePitchBend(uint8_t channel, uint16_t value) {
	}
};

} // namespace a21
