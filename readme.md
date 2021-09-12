# Mega Pad
Basic library for using SEGA Master system and MegaDrive/Genesis 3 and 6 button pads on an arduino with auto detection between the three.

read() can be used to return 8 bit values or one 16-bit value and will always report current buttons. Update allows you to only respond when input is sensed.

Basic usage:
```
#include "MegaPad.h"

const int Up = 2;
const int Down = 3;
const int Left = 4;
const int Right = 5;
const int TL = 6;
const int TR = 8;
const int TS = 7;

int controllerData = 0;     //  x,x,x,x, Z, Y, X, MODE, START, C, B, A, Ri, Lf, Dn, Up

MegaPad pad(Up, Down, Left, Right, TL, TR, TS);     // HardwareSerial port, Sense pin

void setup(){
    Serial.begin(9600);

    // Print pad type
    Serial.print(F("Controller connected: "));
    switch(pad.type()){
        case 0:
        Serial.println(F("SMS"));
        break;
        case 1:
        Serial.println(F("MD3"));
        break;
        case 2:
        Serial.println(F("MD6"));
        break
    }
}

void loop(){
    if(pad.update()){
        // 16 bit read
        controllerData = pad.read();
        Serial.print("Pad: ");
        Serial.println(controllerData, BIN);
    }
}

```

