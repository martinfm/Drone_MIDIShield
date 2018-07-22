
/*
* Copyright (c) 2015 SparkFun Electronics - modified button debounce handler from https://github.com/sparkfun/MIDI_Shield/blob/V_1.5/Firmware/clock-gen/clock-gen.ino 
* Copyright (c) 2018 Fiore Martin
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <MIDI.h>

// CONFIG 
#define MIDI_CHANNEL 16
#define MODWHEEL_CC 1 
#define OCTAVE_LOWER_BOUND 24
#define OCTAVE_UPPER_BOUND 96
#define DEBOUNCE_THRESHOLD 50

// buttons 
#define NOTE_ONOFF_INPUT 4
#define NOTE_OCTAVE_UP 2
#define NOTE_OCTAVE_DOWN 3


// pots
#define NOTE_POT A0
#define MODWHEEL_POT A1 

// leds 
#define NOTE_ONOFF_LED 6
#define OCTAVE_BOUND_REACHED_LED 7

bool note_on = false; 
int note = 0;
int mod_wheel = 0;
int octave = 48; // init to C3, ranges from C1 to C7

// Created and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

bool buttonPressed(int buttonType)
{
  static unsigned int note_onoff_debounce = 0;
  static unsigned int octave_up_debounce = 0;
  static unsigned int octave_down_debounce = 0;
  
  switch(buttonType)
  {
    case(NOTE_ONOFF_INPUT):
    {
      int note_onoff_state = digitalRead(NOTE_ONOFF_INPUT);
      if(note_onoff_state == LOW)
      {
        note_onoff_debounce++;
        if(note_onoff_debounce == DEBOUNCE_THRESHOLD)
        {
          return true;
        }
      }
      else
      {
        note_onoff_debounce = 0;
      }
      break;
    }

    case NOTE_OCTAVE_UP:
    {
      int octave_up_state = digitalRead(NOTE_OCTAVE_UP);
      if(octave_up_state == LOW)
      {
        octave_up_debounce++;
        if(octave_up_debounce == DEBOUNCE_THRESHOLD)
        {
          return true;
        }
      }
      else  
      {
        octave_up_debounce = 0;
      }
      break;
    }

    case NOTE_OCTAVE_DOWN:
    {
      int octave_down_state = digitalRead(NOTE_OCTAVE_DOWN);
      if(octave_down_state == LOW)
      {
        octave_down_debounce++;
        if(octave_down_debounce == DEBOUNCE_THRESHOLD)
        {
          return true;
        }
      }
      else
      {
        octave_down_debounce = 0;
      }
      break;
    }

    default: break;
  }

  return false;
  
}

void sendNoteOn()
{
  MIDI.sendNoteOn(octave+note, 127, MIDI_CHANNEL);
}

void sendNoteOff()
{
  MIDI.sendNoteOff(octave+note, 127, MIDI_CHANNEL);
}

void setup()
{
  pinMode(NOTE_ONOFF_INPUT, INPUT_PULLUP);
  pinMode(NOTE_OCTAVE_UP, INPUT_PULLUP);
  pinMode(NOTE_OCTAVE_DOWN, INPUT_PULLUP);
  
  pinMode(NOTE_ONOFF_LED, OUTPUT);
  pinMode(OCTAVE_BOUND_REACHED_LED, OUTPUT);

  digitalWrite(NOTE_ONOFF_LED, HIGH);
  
  MIDI.begin(MIDI_CHANNEL_OMNI);  
  MIDI.turnThruOff();
}

void loop()
{

  // -- NOTE  
  int new_note = map(analogRead(NOTE_POT), 0, 1023, 0, 11);
  if(new_note != note)
  {
    if(note_on) sendNoteOff();
    note = new_note;
    if(note_on) sendNoteOn();  
  }

  // -- MOD WHEEL  
  int new_modwheel = map(analogRead(MODWHEEL_POT), 0, 1023, 0, 127);
  if(new_modwheel != mod_wheel)
  {
    mod_wheel = new_modwheel;
    MIDI.sendControlChange(MODWHEEL_CC, mod_wheel, MIDI_CHANNEL);
  }

  // -- OCTAVE 
  if(buttonPressed(NOTE_OCTAVE_UP))
  {
    if(octave < OCTAVE_UPPER_BOUND)
    {
      if(note_on) sendNoteOff();
      octave = octave + 12;
      if(note_on) sendNoteOn();
    }
  }

  if(buttonPressed(NOTE_OCTAVE_DOWN))
  {
    if(octave > OCTAVE_LOWER_BOUND)
    {
      if(note_on) sendNoteOff();
      octave = octave - 12;
      if(note_on) sendNoteOn(); 
    }
  }

  // turn red led on if bound reached 
  if(octave == OCTAVE_UPPER_BOUND || octave == OCTAVE_LOWER_BOUND)
  {
    digitalWrite(OCTAVE_BOUND_REACHED_LED, LOW);
  }
  else
  {
    digitalWrite(OCTAVE_BOUND_REACHED_LED, HIGH);
  }
  
  // -- NOTE ON/OFF 
  if(buttonPressed(NOTE_ONOFF_INPUT)) 
  {
    // button pressed toggle note on/off
    note_on = !note_on;
    if(note_on)
    {
      sendNoteOn();  
    }
    else
    {
      sendNoteOff();  
    }
  }
  digitalWrite(NOTE_ONOFF_LED, note_on ? LOW : HIGH);

  
}

