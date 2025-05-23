/**
 * @file IRremote.h
 *
 * @brief Stub for backward compatibility
 */

#ifndef IRremote_h
#define IRremote_h

#include "IRremote.hpp"

#warning It seems, that you are using an old version 2.0 code / example.
#warning This version is no longer supported!
#warning Upgrade instructions can be found here: "https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#converting-your-2x-program-to-the-4x-version"
#warning Please use one of the new code examples from the library, available at "File > Examples > Examples from Custom Libraries / IRremote".
#warning Start with the SimpleReceiver or SimpleSender example.
#warning The examples are documented here: "https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#examples-for-this-library"
#warning Or just downgrade your library to version 2.6.0.

/**********************************************************************************************************************
 * The OLD and DEPRECATED decode function with parameter aResults, kept for backward compatibility to old 2.0 tutorials
 * This function calls the old MSB first decoders and fills only the 3 variables:
 * aResults->value
 * aResults->bits
 * aResults->decode_type
 * It prints a message on the first call.
 **********************************************************************************************************************/
bool IRrecv::decode(decode_results *aResults) {
    static bool sMessageWasSent = false;
    if (!sMessageWasSent) {
        Serial.println(F("**************************************************************************************************"));
        Serial.println(F("Thank you for using the IRremote library!"));
        Serial.println(F("It seems, that you are using an old version 2.0 code / example."));
        Serial.println(F("This version is no longer supported!"));
        Serial.println();
        Serial.println(F("Upgrade instructions can be found here:"));
        Serial.println(F(" https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#converting-your-2x-program-to-the-4x-version"));
        Serial.println();
        Serial.println(F("Please use one of the new code examples from the library,"));
        Serial.println(F(" available at \"File > Examples > Examples from Custom Libraries / IRremote\"."));
        Serial.println(F("Start with the SimpleReceiver or SimpleSender example."));
        Serial.println(F("The examples are documented here:"));
        Serial.println(F(" https://github.com/Arduino-IRremote/Arduino-IRremote?tab=readme-ov-file#examples-for-this-library"));
        Serial.println();
        Serial.println(F("Or just downgrade your library to version 2.6.0."));
        Serial.println();
        Serial.println(F("Thanks"));
        Serial.println(F("**************************************************************************************************"));
        Serial.println();
        Serial.println();
        sMessageWasSent = true;
    }
    return decode_old(aResults);
}

#endif // IRremote_h
#pragma once

