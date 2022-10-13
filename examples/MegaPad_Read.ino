/*
   MegaPad Read demo
   NKS 2021
   =================
   The MegaPad library can auto detect between Atari/SMS pads, MegaDrive 3 button pads
   and MegaDrive 6 button pads. The library returns either two bytes or a single integer
   where all the buttons are mapped to specific bits. They're arranged as follows:

   Byte 0, MSB B7-B0:
   [Start, C, B, A, Right, Left, Down, Up]

   Byte 1 (or high byte of int)
   MSB B7-B0
   [N/A, N/A, N/A, N/A, Mode, Z, Y, X]

   Initiallizer needs to know what pins on the DE-9 connector are connected to.
   Sega DE9 pinout looking at pad connector (female):

     5  4  3  2  1
   _________________
   | o  o  o  o  o |
    \__o__o__o__o /

       9  8  7  6

   1 = Up
   2 = Down
   3 = Left
   4 = Right
   5 = 5 Volt power
   6 = Trigger Left (B when select is not active)
   7 = Trigger Select (changes whether B and C or A and Start are sent on TL and TR)
   8 = Ground
   9 = Trigger Right (C when select is not active)
*/

#include "MegaPad.h"

enum Controllers {SMS, MD3, MD6};
Controllers padType;

const byte UP = 2;
const byte DN = 3;
const byte LF = 4;
const byte RG = 5;
const byte TL = 6;
const byte TR = 7;
const byte TS = 8;

// Initializer
MegaPad pad(UP, DN, LF, RG, TL, TR, TS);

uint16_t lastButtons = 0;
uint16_t padButtons = 0;

// This holds the last poll time. This is used as reading quickly can lead to multiple triggers.
uint32_t padDebounce = 0;
uint32_t padTimer = 0;

String buttons[3][12] = {
  {"Up", "Down", "Left", "Right", "", "1", "2", "", "", "", "", ""},
  {"Up", "Down", "Left", "Right", "A", "B", "C", "Start", "", "", "", ""},
  {"Up", "Down", "Left", "Right", "A", "B", "C", "Start", "X", "Y", "Z", "Mode"}
};

void setup() {
  Serial.begin(9600);
  Serial.println("MegaPad Read Demo");

  //padType is an enum that has the states of SMS, MD3 or MD6, for MasterSystem, 3 button and 6 putton types.
  // If select pin is not given, or no pad is connected, the library will default to SMS mode
  padType = pad.type();

  Serial.print("Controller type: ");
  switch (padType) {
    case SMS:
      Serial.println("SMS");
      break;

    case MD3:
      Serial.println("MD3");
      break;

    case MD6:
      Serial.println("MD6");
      break;
  }
}

void loop() {
  // Get new buttons
  padButtons = pad.read();

  // Read the new button states if there are any
  if (padButtons != lastButtons && (millis() - padDebounce >= 10)) {
    // Store the time to stop bouncing
    padDebounce = millis();

    // This prints the binary representation of the controller buttons
    Serial.print('B');
    Serial.print(padButtons, BIN);
    Serial.print(" - ");
    // This prints the names of the buttons pressed
    bool buttonPressed = false;
    for (int p = 0; p < 12; p++) {
      if (bitRead(padButtons, p) == true) {
        if (buttonPressed == true) {
          Serial.print(", ");
        }
        Serial.print(buttons[padType][p]);
        buttonPressed = true;
      }
    }
    Serial.println();

    // Store the current button states to compare
    lastButtons = padButtons;
  }

  // Auto detect a pad being changed. This would be better to do as a timed function in real world uses
  //  to keep the overhead down but for this example this will work fine
  // We need to do this when the buttons aren't pressed or you will get glitched results.
  //  this is because we know that if the pad is removed, then no buttons are pressed.
  if (millis() - padTimer >= 250 && padButtons == 0) {
    padTimer = millis();
    Controllers test = pad.type();
    if (padType != test) {
      padType = test;
      Serial.print("Controller type changed: ");
      switch (padType) {
        case SMS:
          Serial.println("SMS");
          break;

        case MD3:
          Serial.println("MD3");
          break;

        case MD6:
          Serial.println("MD6");
          break;
      }
    }
  }
}
