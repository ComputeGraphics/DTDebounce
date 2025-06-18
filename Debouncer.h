#ifndef DTDebouncer_h
#define DTDebouncer_h

#include "Arduino.h"

class DTDebouncer
{
    public:
        //Instant Delay Debouncer
        void debounce(uint8_t pin, bool pullup, uint8_t delay, void (*handler)());
        void debounceHold(uint8_t pin, bool pullup, uint8_t delay, uint32_t hold, void (*handlerClick)(), void (*handlerHold)());
        //Threshold Debouncer
        void tdebounce(uint8_t pin, bool pullup, uint16_t threshold, uint16_t delay, void (*handler)());
        void tdebounceHold(uint8_t pin, bool pullup, uint16_t threshold, uint16_t thresholdHold, uint16_t delay, void (*handlerClick)(), void (*handlerHold)());
        // !! Click Debouncers call their handler directly after TRIGGERED to reduce latency !!
        // !! Hold Debouncers call their Click Handler after the button has been RELEASED    !!

        //Metrics (Optional)
        uint32_t microsClick = 0;
        uint32_t microsRelease = 0;
        uint32_t clickTime = 0;
        //Metrics (Necessary)
        uint32_t cyclesCount = 0;       // Click Mode: Cycles Count inside Timeout
                                        // Hold Mode:  How many cycles the button was held after triggering
        bool isTimeout = false;         // Button is Timeout - True when button is down (Debounced)
        bool buttonHold = false;        // True when the last button action triggered the onHold Handler
        uint8_t bounceCount = 0;        // How many times the button bounced - NA in TDebounce

        //Settings
        bool statsSerial = false;       // Output Statistics of Button Click to Serial after Relase (DEBUG)
        // !! Causes major latency differences because serial output takes a lot of time. Consider reducing delay when activating !!s

        //Delay Mode
        uint8_t minimumOff = 2;         // How many cycles the button has to be off to be released virtually
        
        //Threshold Mode
        uint8_t filterTolerance = 4;    // How much the threshold can be overrun to compensate wobbling
                                        // Higher values mean the button will take longer to be released virtually!
        bool thresholdReached = false;  // If the threshold has been reached in the last recognized button action

    private:
        int offCount = 0;               // offCounter is a Multipurpose Counter Variable
        uint32_t pendingCheck = 0;      // pendingCheck + delay equals the time where next buttonState check
        void reset();                   // release the timeout and collect metrics
};

#endif
