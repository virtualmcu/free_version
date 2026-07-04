// BluePill STM32F103 — main.c (Simple API)
// Arduino-style: see bluepill.h for the full set of functions.
//
#include "bluepill.h"

int main(void) {
    pinMode(LED_BUILTIN, OUTPUT);   // PC13 = on-board LED
    Serial.begin(115200);
    Serial.println("BluePill ready");

    while (1) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("OFF");        // active-low: HIGH = LED off
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("ON");
        delay(1000);
    }
}
