#include <Arduino.h>
#include "TimerEvent.h"

TimerEvent t1 = TimerEvent();

void itsAlive()
{
    digitalWrite(13, 1);
    delay(66);
    digitalWrite(13, 0);
}

void setup()
{
    pinMode(13, OUTPUT);
    t1.set(1111, itsAlive);
}

void loop()
{
    t1.update();
}
