#include <Arduino.h>
#include "OTALIB.h"

const char* ssid = "Name-wifi";
const char* password = "password";
const char* username = "username";

MyOTA myOTA(ssid, password, username);

void setup() {
    myOTA.begin();
}

void loop() {
    myOTA.loop();
}
