#include "Arduino.h"
#include "Debouncer.h"

/*uint8_t offCount = 0;
public: uint32_t cyclesCount = 0;
public: bool isTimeout = false;
public: uint32_t microsClick = 0;    //OPT
public: uint32_t microsRelease = 0;  //OPT
uint32_t pendingCheck = 0;
public: uint32_t clickTime = 0;  //OPT
public: bool buttonHold = false;
public: uint8_t bounceCount = 0;*/

void Debouncer::reset()
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

// Debounce over delay
void Debouncer::debounce(uint8_t pin, bool pullup, uint8_t delay, void (*handler)())
{
  bool state = digitalRead(pin);
  if ((state != pullup) && !isTimeout) // && nicht im timeout ist
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
    // Serial.println("Current Time: " + microsCurrent);
    if (pendingCheck + delay <= microsCurrent)
    {
      // Serial.println("Check Running");
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

void Debouncer::debounceHold(uint8_t pin, bool pullup, uint8_t delay, uint32_t hold, void (*handlerClick)(), void (*handlerHold)())
{
  bool state = digitalRead(pin);
  if ((state != pullup) && !isTimeout) // && nicht im timeout ist
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
        // Serial.println(cyclesCount);
        if (cyclesCount == hold && !buttonHold)
        {
          buttonHold = true;
          handlerHold();
        }
      }
      pendingCheck = micros();
    }

    // Button wird geklickt - LOW   -> Sofort Handler callen
    // Timeout bis zum nächsten HIGH, das länger als 10ms anhält

    // Nach delay Micros button checken, ob sich der zustand geändert hat
    // Die wartezeit des letzten cycles wird mit 2 multipliziert
    // Ja -> Noch einen Zyklus durchführen.
    // Nein -> nach 2 Zyklen freigeben
  }
}

// Debounce Low-Pass
// offCount als threshold variable erneut verenden
// buttonHold als marker recyclen
// Alle delay mikrosekunden wird threshold um 1 subtrahiert, wenn button aus und umgekehrt
// Button Handler wird ausgeführt wenn threshold erreicht wird
void Debouncer::tdebounce(uint8_t pin, bool pullup, uint16_t threshold, uint16_t delay, void (*handler)())
{
  bool state = digitalRead(pin);
  if ((state != pullup) && !isTimeout) // && nicht im timeout ist
  {
    bounceCount = 0;
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

      //Serial.println(String(offCount) + " : " + String(threshold));
      if (offCount == threshold && !buttonHold)
      {
        //Serial.println("Threshold Reached");
        buttonHold = true;
        handler();

        return;
      }
      if (offCount <= threshold && buttonHold)
      {
        buttonHold = false;
        reset();
      }
    }
    cyclesCount++;
    pendingCheck = micros();
  }
}
