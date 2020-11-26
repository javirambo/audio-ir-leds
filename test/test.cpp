#include <Arduino.h>

void itsAlive()
{
    static unsigned long timer = 0;
    digitalWrite(13, timer < 1000);
    if (++timer > 300000L)
        timer = 0;
}

void setup()
{
    pinMode(13, OUTPUT);
}

void loop()
{
    itsAlive();
}
