#include <Arduino.h>

#include "../include/constants.h"
#include "../include/wifi.h"
#include "../include/cubecontrol.h"

CubeController cController(BAUDRATE);
WifiController wifiController(WIFI_SSID, WIFI_PASSWORD);
WebServer websrv(80);

void handleRoot()
{
    websrv.send(200, "text/html", WEBSITE_HTML);
}

void setClockTime()
{
    String clockSet = websrv.arg("plain");
    cController.setBaseTime(clockSet.toInt());

    websrv.send(200, "text/plain", "New Time Set");
}

void setClockEnabled()
{
    String clockSet = websrv.arg("plain");
    clockSet.toLowerCase();
    cController.setClockEnabled(clockSet == "true");

    websrv.send(200, "text/plain", "Mode updated");
}

void handleDisplay()
{
    String frameRaw = websrv.arg("plain");
    cController.sendFrame(frameRaw.c_str(), frameRaw.length());
    websrv.send(200, "text/plain", "Frame send to cube");
}

void handleNotFound()
{
    websrv.send(404, "text/plain", "404 Not Found");
}

void initWebServer()
{
    websrv.on("/", HTTP_GET, handleRoot);
    websrv.on("/display", HTTP_POST, handleDisplay);
    websrv.on("/setclock/time", HTTP_POST, setClockTime);
    websrv.on("/setclock/enabled", HTTP_POST, setClockEnabled);
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
    cController.update();
}