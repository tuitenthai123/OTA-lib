#ifndef OTALIB
#define OTALIB

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ElegantOTA.h>
#include <PubSubClient.h>

class MyOTA {
public:
    MyOTA(const char* ssid, const char* password, const char* username);
    void begin();
    void loop();

private:
    const char* ssid;
    const char* password;
    const char* username;
    const char* mqttServer = "broker.hivemq.com";
    const int mqttPort = 1883;
    const char* mqttTopic = "VNPT/OTA_STATUS";
    const char* mqttClientId = "ota_device";

    WiFiClient espClient;
    PubSubClient mqttClient;
    WebServer server;

    unsigned long ota_progress_millis = 0;
    String updateFileName = "";

    void connectToMQTT();
    void onOTAStart();
    void onOTAProgress(size_t current, size_t final);
    void onOTAEnd(bool success);
    String generateRandomFileName();
};

#endif