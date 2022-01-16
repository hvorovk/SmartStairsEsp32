#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Private header should have two char* constansts
#include "../private.h"
// For example
// const char* ssid = "";
// const char* password = "";

/**
 * Standart Ota procedure with wifi setup
 */

const long utcOffsetInSeconds = 7 * 60 * 60;

WiFiUDP ntpUDP;

void otaTask( void * parameter) {

    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(ssid, password);

   
    WiFi.waitForConnectResult();
    while (!WiFi.isConnected()) {
        Serial.println("No wifi mode!");
        delay(1000);
        WiFi.waitForConnectResult();
    }

    ArduinoOTA.setPort(8080);
    ArduinoOTA.setHostname("Stairs32");

    ArduinoOTA
    .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
    })
    .onEnd([]() {
        Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();

    Serial.println("OTA Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    for (;;) {
        ArduinoOTA.handle();
        delay(200); // Can be replaced for another value
    }
}