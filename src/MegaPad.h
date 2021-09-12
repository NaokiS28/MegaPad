/*
 * MegaPAD
 * =======
 * NKS 2019
 * 
 * Library for reading any and all Mega Drive style 9-Pin DE-9 connector controllers. May expand to Saturn controllers later.
 * This does not use the Mega/Sega Mouse. The code will auto detect the installed pad or default to SMS mode (4D 2B).
 * Library can be used in either an interrupt-style (update returns whether buttons changed) or polled style (always read current buttons)
 * 
 * Modes supported:
 *  SMS = 4 Direction, 2 Button
 *  MD3 = 4 Direction, 3 Button + Start
 *  MD6 = 4 Direction, 6 Button + Start & Mode
 *  
 */
 
#ifndef MEGA_PAD_H
#define MEGA_PAD_H

#include <Arduino.h>


class MegaPad{
  private:
  bool noMD = false;    // If no TH (select) pin is given, then mega drive modes are disabled
  uint8_t padPin[7];
  uint8_t padMode = 0;  // 0 = SMS/Unconfigured, 1 = MD, 2 = MD6
  uint8_t padButtons[2];  // [0] = SCBARLDU, [1] = xxxxMZYX
  uint8_t padHistory[2];
  
  void readRaw();       // Read the current 6-bit port into padButtons
  void detectType();    // Try and attempt to autodetect connected pad. Will default to SMS.

  void readPad();       // Forwarder to select the following
  void readMD();
  void readMD6();
  void readSMS();
  bool ignorePadCheck = false;   // When set true, readSMS/MD/MD6 will not change the controller type back if anomolies are detected
  
  public:
  // Initializer, if d_s is not specified, only SMS mode will be used.
  MegaPad (uint8_t d_u, uint8_t d_d, uint8_t d_l, uint8_t d_r, uint8_t d_a, uint8_t d_b, uint8_t d_s = 255);
  bool interrupt = false; // Soft interrupt to say pad state changed
  
  void setMode(uint8_t m);  // Can force set the type of controller if desired. This can be used to set between SMS and MD.
  bool update();    // Optional but can be used to always poll the controller, polled when buttons are read aswell.
  uint8_t readDirection();  // Returns the D-Pad. Even numbers are single cardinal directions, odd are two cardinals pressed.
                            // 0 means no direction pressed.
  bool readStart(); // True if Start button is pressed.
  uint8_t type();   // Returns wether SMS, MD or MD6
  uint8_t read(uint8_t b);
  uint16_t read();  // Returns all buttons. Any unused buttons are always cleared (XYZM if in MD for example) 
  
};

#endif
