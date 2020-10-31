#include <Arduino.h>

#include "sm.stairs.h"
#include "Network/OtaTask.h"

SMStairs stairs;


void setup() {
    Serial.begin(115200);

    Serial.println("Booting");
    
    stairs.init({
        14,
        15,
        17,
        23,
        23,
        21,
        21,
        23,
        25,
        20,
        17,
        17,
        17,
        17,
        18
    });

    // Run OTA in second thread
    xTaskCreatePinnedToCore(
        otaTask, /* Function to implement the task */
        "Ota", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        0,  /* Priority of the task */
        NULL,  /* Task handle. */
        (xPortGetCoreID() + 1) % 2   /* Core where the task should run */
    );
}

void loop() {
    stairs.makeStep();
}
