#include "OTALIB.h"

MyOTA::MyOTA(const char* ssid, const char* password, const char* username)
    : ssid(ssid), password(password), username(username), mqttClient(espClient), server(80) {}

void MyOTA::begin() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println("");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Connect to MQTT
    mqttClient.setServer(mqttServer, mqttPort);
    connectToMQTT();

    // Start HTTP server
    server.on("/", [this]() {
        server.send(200, "text/plain", "Hi! This is ElegantOTA Demo.");
    });

    ElegantOTA.begin(&server);
    ElegantOTA.onStart([this]() { onOTAStart(); });
    ElegantOTA.onProgress([this](size_t current, size_t final) { onOTAProgress(current, final); });
    ElegantOTA.onEnd([this](bool success) { onOTAEnd(success); });

    server.begin();
    Serial.println("HTTP server started");
}

void MyOTA::loop() {
    server.handleClient();
    ElegantOTA.loop();
    if (!mqttClient.connected()) {
        connectToMQTT();
    }
    mqttClient.loop();
}

void MyOTA::connectToMQTT() {
    while (!mqttClient.connected()) {
        Serial.print("\nConnecting to MQTT...");
        if (mqttClient.connect(mqttClientId)) {
            Serial.println("Connected to MQTT!");
        } else {
            Serial.print("Failed, error code = ");
            Serial.println(mqttClient.state());
            delay(5000);
        }
    }
}

void MyOTA::onOTAStart() {
    Serial.println("OTA update started!");

    if (server.hasHeader("Content-Disposition")) {
        String contentDisposition = server.header("Content-Disposition");
        int filenameIndex = contentDisposition.indexOf("filename=");
        if (filenameIndex != -1) {
            updateFileName = contentDisposition.substring(filenameIndex + 9); // Extract file name
            updateFileName.trim();

            if (updateFileName.startsWith("\"") && updateFileName.endsWith("\"")) {
                updateFileName = updateFileName.substring(1, updateFileName.length() - 1); // Remove quotes
            }
        }
    }

    // Nếu không lấy được tên file từ header, tạo tên ngẫu nhiên
    if (updateFileName == "") {
        updateFileName = generateRandomFileName() + ".bin";
    }

    Serial.print("OTA file name: ");
    Serial.println(updateFileName);
}

void MyOTA::onOTAProgress(size_t current, size_t final) {
    if (millis() - ota_progress_millis > 1000) {
        ota_progress_millis = millis();
        Serial.printf("OTA Progress: %u/%u bytes\n", current, final);
    }
}

void MyOTA::onOTAEnd(bool success) {
    if (success) {
        Serial.println("OTA update finished successfully!");

        // Send info to MQTT
        String macAddress = WiFi.macAddress();
        String filename = ElegantOTA.getUploadedFilename();
        String fileid = ElegantOTA.getIDBinFile();
        String payload = String("{") +
                         "\"username\": \"" + username + "\"," +
                         "\"macAddress\": \"" + macAddress + "\"," +
                         "\"file\": \"" + filename.c_str() + "\"," +
                         "\"fileid\": \"" + fileid + "\"," +
                         "\"fileurl\": \"https://drive.google.com/file/d/" + fileid + "/view?usp=sharing\"" +
                         "}";
        mqttClient.publish(mqttTopic, payload.c_str());
        Serial.print("MQTT topic sent: ");
        Serial.println(mqttTopic);
        Serial.print("MQTT payload: ");
        Serial.println(payload);
    } else {
        Serial.println("There was an error during OTA update!");
    }
}

String MyOTA::generateRandomFileName() {
    String randomFileName = "";
    for (int i = 0; i < 6; i++) {
        char randomChar = 'a' + random(0, 26); // Random char from 'a' to 'z'
        randomFileName += randomChar;
    }
    return randomFileName;
}