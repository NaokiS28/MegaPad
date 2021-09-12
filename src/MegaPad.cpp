/*
   MegaPAD
   =======
   NKS 2019

   Library for reading any and all Mega Drive style 9-Pin DE-9 connector controllers. May expand to Saturn controllers later.
   This does not use the Mega/Sega Mouse. The code will auto detect the installed pad or default to SMS mode (4D 2B).
   Library can be used in either an interrupt-style (update returns whether buttons changed) or polled style (always read current buttons)

   Modes supported:
    SMS = 4 Direction, 2 Button
    MD3 = 4 Direction, 3 Button + Start
    MD6 = 4 Direction, 6 Button + Start & Mode

*/

#ifndef MEGA_PAD_C
#define MEGA_PAD_C

#define PAD_U 0
#define PAD_D 1
#define PAD_L 2
#define PAD_R 3
#define PAD_A 4
#define PAD_B 5
#define PAD_S 6

#include "MegaPad.h"

// Initializer, if d_s is not specified, only SMS mode will be used.
MegaPad::MegaPad (uint8_t d_u, uint8_t d_d, uint8_t d_l, uint8_t d_r, uint8_t d_a, uint8_t d_b, uint8_t d_s) {
  padPin[0] = d_u;
  padPin[1] = d_d;
  padPin[2] = d_l;
  padPin[3] = d_r;
  padPin[4] = d_a;
  padPin[5] = d_b;
  padPin[6] = d_s;
  
  for (uint8_t p = 0; p < 6; p++) {
    pinMode(padPin[p], INPUT_PULLUP);
  }

  if (d_s != 255) {
    noMD = false;
    pinMode(padPin[6], OUTPUT);
    detectType();
  } else {
    padMode = 0;
    noMD = true;
  }
}

void MegaPad::readSMS() {
  memcpy(padHistory, padButtons, sizeof(padHistory));
  memset(padButtons, 0x00, sizeof(padButtons));
  digitalWrite(padPin[PAD_S], HIGH);	// Make sure we're only reading the first buttons
  bitWrite(padButtons[0], 0, !digitalRead(padPin[PAD_U]));
  bitWrite(padButtons[0], 1, !digitalRead(padPin[PAD_D]));
  bitWrite(padButtons[0], 2, !digitalRead(padPin[PAD_L]));
  bitWrite(padButtons[0], 3, !digitalRead(padPin[PAD_R]));
  bitWrite(padButtons[0], 5, !digitalRead(padPin[PAD_A]));  // B
  bitWrite(padButtons[0], 6, !digitalRead(padPin[PAD_B]));  // C
}

void MegaPad::readMD() {
  readSMS();
  digitalWrite(padPin[PAD_S], LOW);	// Read second set of buttons
  //if(ignorePadCheck == true || (digitalRead(padPin[PAD_L]) == LOW && digitalRead(padPin[PAD_R]) == LOW)) {
  delayMicroseconds(150);
  bitWrite(padButtons[0], 4, !digitalRead(padPin[PAD_A]));  // A
  bitWrite(padButtons[0], 7, !digitalRead(padPin[PAD_B]));  // Start
  digitalWrite(padPin[PAD_S], HIGH);	// Read second set of buttons
}

void MegaPad::readMD6() {
  readMD();	// Why not save time?
  delayMicroseconds(150);
  digitalWrite(padPin[PAD_S], LOW);
  delayMicroseconds(150);
  digitalWrite(padPin[PAD_S], HIGH);
  delayMicroseconds(150);
  digitalWrite(padPin[PAD_S], LOW);
  /*if(ignorePadCheck == true || (digitalRead(padPin[PAD_L]) == LOW && digitalRead(padPin[PAD_R]) == LOW && digitalRead(padPin[PAD_U]) == LOW && digitalRead(padPin[PAD_D]) == LOW)) {*/
  digitalWrite(padPin[PAD_S], HIGH); // Should be in MD6 mode for supported controllers
  bitWrite(padButtons[1], 0, !digitalRead(padPin[PAD_L]));
  bitWrite(padButtons[1], 1, !digitalRead(padPin[PAD_D]));
  bitWrite(padButtons[1], 2, !digitalRead(padPin[PAD_U]));
  bitWrite(padButtons[1], 3, !digitalRead(padPin[PAD_R]));

  digitalWrite(padPin[PAD_S], HIGH);	// Read second set of buttons
}

void MegaPad::detectType() {
  padMode = 0;
  digitalWrite(padPin[PAD_S], HIGH);	// Release TH (Select)
  delayMicroseconds(3000);
  digitalWrite(padPin[PAD_S], LOW);	// Assert TH (Select)
  if (digitalRead(padPin[PAD_L]) == LOW && digitalRead(padPin[PAD_R]) == LOW) {
    padMode = 1;	// Is atleast an MD 3-button.
    delayMicroseconds(150);
    digitalWrite(padPin[PAD_S], HIGH);	// Assert TH (Select)
    delayMicroseconds(150);
    digitalWrite(padPin[PAD_S], LOW);	// Assert TH (Select)
    delayMicroseconds(150);
    digitalWrite(padPin[PAD_S], HIGH);	// Assert TH (Select)
    delayMicroseconds(150);
    digitalWrite(padPin[PAD_S], LOW);	// Assert TH (Select)
    delayMicroseconds(150);		// Should be in MD6 mode for supported controllers
    //readRaw();
    if (digitalRead(padPin[PAD_L]) == LOW && digitalRead(padPin[PAD_R]) == LOW && digitalRead(padPin[PAD_U]) == LOW && digitalRead(padPin[PAD_D]) == LOW) {
      padMode = 2;	// Button responses indicate 6 button is present
    }
  }
  digitalWrite(padPin[PAD_S], HIGH);
}

void MegaPad::readRaw() {
  // Read the current 6-bit port with no processing
  memcpy(padHistory, padButtons, sizeof(padHistory));
  bitWrite(padButtons[0], 0, digitalRead(padPin[PAD_U]));
  bitWrite(padButtons[0], 1, digitalRead(padPin[PAD_D]));
  bitWrite(padButtons[0], 2, digitalRead(padPin[PAD_L]));
  bitWrite(padButtons[0], 3, digitalRead(padPin[PAD_R]));
  bitWrite(padButtons[0], 4, digitalRead(padPin[PAD_A]));
  bitWrite(padButtons[0], 5, digitalRead(padPin[PAD_B]));
}

void MegaPad::readPad() {
  // Select and read pad type
  static unsigned long CT;
  static unsigned long DB;

  CT = millis();
  if ((CT - DB) >= 5) {
    DB = CT;
    switch (padMode) {
      case 0:
        readSMS();
        break;
      case 1:
        readMD();
        break;
      case 2:
        readMD6();
        break;
      default:
        break;
    }
    // If padbuttons doesn't match padHistory, then assume buttons changed state
    if (memcmp(padHistory, padButtons, sizeof(padHistory)) != 0) {
      interrupt = true;
    }
  }
}

bool MegaPad::update() {
  // Poll pad and return true if changes detected
  static unsigned long CT;
  static unsigned long DB;

  CT = millis();
  if ((CT - DB) >= 10) {
    DB = CT;
    //detectType();
    readPad();	// Use forwarder to select pad type
  }
  return interrupt;
}

uint8_t MegaPad::readDirection() {
  //detectType();
  readPad();
  uint8_t dir = (padButtons[0] & 0x0F);
  interrupt = false;
  return dir;
}

bool MegaPad::readStart() {
  //detectType();
  readPad();
  bool st = bitRead(padButtons[0], 7);
  interrupt = false;
  return st;
}

uint8_t MegaPad::type() {
  detectType();
  return padMode;
}

uint16_t MegaPad::read() {
  //detectType();
  readPad();
  unsigned int word = padButtons[1] * 256 + padButtons[0];
  interrupt = false;
  return word;
}

uint8_t MegaPad::read(uint8_t m){
  readPad();
  interrupt = false;
  return padButtons[m];
}

void MegaPad::setMode(uint8_t m) {
  // Don't change mode if there is no select (TH) pin
  if (!noMD) {
    padMode = m;
    ignorePadCheck = true;
  }
}

#endif
