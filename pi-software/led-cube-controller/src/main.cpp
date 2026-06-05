#include <Arduino.h>

#include "../include/constants.h"
#include "../include/wifi.h"
#include "../include/cubecontrol.h"

CubeController cController(BAUDRATE);
WifiController wifiController(WIFI_SSID, WIFI_PASSWORD);
WebServer websrv(80);

void handleRoot()
{
    String frameRaw = websrv.arg("plain");
    Serial.println(frameRaw);
    Serial.println("y");
    cController.sendFrame(frameRaw.c_str(), frameRaw.length());
    websrv.send(200, "text/plain", "Frame send to cube");
}

void handleNotFound()
{
    websrv.send(404, "text/plain", "404 Not Found");
}

void initWebServer()
{
    websrv.on("/", HTTP_POST, handleRoot);
    websrv.onNotFound(handleNotFound);
    websrv.begin();
}

void setup()
{
    // Serial1.begin(9600L);
    Serial1.begin(BAUDRATE, SERIAL_8N1);
    Serial.begin(115200);
    wifiController.tryUntilWifiConnected();
    initWebServer();
    Serial.println(WiFi.localIP());
}

void loop()
{
    websrv.handleClient();
}