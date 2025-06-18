#include "Arduino.h"
#include "DTDebouncer.h"

void DTDebouncer::reset()
{
  uint32_t micNow = micros();
  clickTime = micNow - microsClick;
  pendingCheck = 0;
  offCount = 0;
  microsRelease = micNow;
  isTimeout = false;
  if (statsSerial)
  {
    Serial.println("Button Released");
    Serial.println("Clicked Length: " + String(clickTime));
    Serial.println("Click Time: " + String(microsClick));
    Serial.println("Release Time: " + String(microsRelease));
    Serial.println("Bounce Count: " + String(bounceCount));
    Serial.println("Button was held: " + String(buttonHold));
    Serial.println("Sensor Cycles: " + String(cyclesCount));
  }
}

//Call the handler on the first flank for latency
//Then Timeout the button until its physically released again
void DTDebouncer::debounce(uint8_t pin, bool pullup, uint8_t delay, void (*handler)())
{
  bool state = digitalRead(pin);
  if ((state != pullup) && !isTimeout)
  {
    bounceCount = 0;
    microsClick = 0;
    cyclesCount = 0;
    isTimeout = true;
    microsClick = micros();
    pendingCheck = microsClick;
    handler();
    return;
  };
  if (isTimeout)
  {
    uint32_t microsCurrent = micros();
    if (pendingCheck + delay <= microsCurrent)
    {
      if (state == pullup)
      {
        if (offCount >= minimumOff)
        {
          reset();
          return;
        }
        offCount++;
      }
      else
      {
        if (offCount > 0)
          bounceCount++;
        offCount = 0;
      }
      cyclesCount++;
      pendingCheck = micros();
    }
  }
}

//Actually its just the same but after the hold cycle value is reached something cool happens
void DTDebouncer::debounceHold(uint8_t pin, bool pullup, uint8_t delay, uint32_t hold, void (*handlerClick)(), void (*handlerHold)())
{
  bool state = digitalRead(pin);
  if ((state != pullup) && !isTimeout)
  {
    bounceCount = 0;
    microsClick = 0;
    cyclesCount = 0;
    buttonHold = false;
    isTimeout = true;
    microsClick = micros();
    pendingCheck = microsClick;
    return;
  };
  if (isTimeout)
  {
    uint32_t microsCurrent = micros();

    if (pendingCheck + delay <= microsCurrent)
    {
      if (state == pullup)
      {
        if (offCount >= minimumOff)
        {
          reset();
          if (!buttonHold)
            handlerClick();
          return;
        }
        offCount++;
      }

      if (state != pullup)
      {
        if (offCount > 0)
          bounceCount++;
        offCount = 0;
        cyclesCount++;
        if (cyclesCount == hold && !buttonHold)
        {
          buttonHold = true;
          handlerHold();
        }
      }
      pendingCheck = micros();
    }
  }
}

// The idea is to simulate a low-pass filter and offCount is our capacitor
// Everytime the button is recognized pressed the content increases and the other way round
// If the given threshold is reached the handler gets called
// The button will be released after the offCount falls below the threshold
void DTDebouncer::tdebounce(uint8_t pin, bool pullup, uint16_t threshold, uint16_t delay, void (*handler)())
{
  bool state = digitalRead(pin);
  if ((state != pullup) && !isTimeout)
  {
    microsClick = 0;
    cyclesCount = 0;
    isTimeout = true;
    microsClick = micros();
    pendingCheck = microsClick;
    return;
  }

  if (isTimeout)
  {
    uint32_t microsCurrent = micros();
    if (pendingCheck + delay <= microsCurrent)
    {
      if(state == pullup && offCount > 0) offCount--;
      else if(state != pullup && offCount < threshold+filterTolerance) offCount++;
      if (offCount == threshold && !thresholdReached)
      {
        thresholdReached = true;
        handler();
        return;
      }
      if (offCount <= threshold && thresholdReached)
      {
        thresholdReached = false;
        reset();
      }
      cyclesCount++;
      pendingCheck = micros();
    }
  }
}

//The same again but with another threshold at the value the button should be considered as being held down
void DTDebouncer::tdebounceHold(uint8_t pin, bool pullup, uint16_t threshold, uint16_t thresholdHold, uint16_t delay, void (*handlerClick)(), void (*handlerHold)())
{
  bool state = digitalRead(pin);
  if ((state != pullup) && !isTimeout)
  {
    buttonHold = false;
    microsClick = 0;
    cyclesCount = 0;
    isTimeout = true;
    microsClick = micros();
    pendingCheck = microsClick;
    return;
  }

  if (isTimeout)
  {
    uint32_t microsCurrent = micros();
    if (pendingCheck + delay <= microsCurrent)
    {
      if(state == pullup && offCount > 0) offCount--;
      else if(state != pullup && offCount < thresholdHold+filterTolerance) offCount++;
      if (offCount == threshold && !thresholdReached)
      {
        thresholdReached = true;
        return;
      }
      if (offCount == thresholdHold && thresholdReached && !buttonHold)
      {
        handlerHold();
        buttonHold = true;
        return;
      }
      if (offCount < thresholdHold && offCount > threshold) cyclesCount++;
      if ((offCount <= threshold && thresholdReached) || (offCount <= thresholdHold && buttonHold))
      {
        if(!buttonHold) handlerClick();
        thresholdReached = false;
        reset();
      }
    }
    pendingCheck = micros();
  }
}
