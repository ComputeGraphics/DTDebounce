#ifndef Debouncer_h
#define Debouncer_h

#include "Arduino.h"

class Debouncer
{
    public:
        void debounce(uint8_t pin, bool pullup, uint8_t delay, void (*handler)());
        void debounceHold(uint8_t pin, bool pullup, uint8_t delay, uint32_t hold, void (*handlerClick)(), void (*handlerHold)());
        void tdebounce(uint8_t pin, bool pullup, uint16_t threshold, uint16_t delay, void (*handler)());

        uint32_t cyclesCount = 0;
        bool isTimeout = false;
        uint32_t microsClick = 0;    //OPT
        uint32_t microsRelease = 0;  //OPT
  
        uint32_t clickTime = 0;  //OPT
        bool buttonHold = false; // in TDebounce, ob der Threshold wurde erreicht wurde
        uint8_t bounceCount = 0;

        bool statsSerial = false;
        uint8_t minimumOff = 1;
        uint8_t filterTolerance = 4;
    private:
        int offCount = 0; //in TDebounce aktueller Zählerstand
        uint32_t pendingCheck = 0;
        void reset();
};

#endif