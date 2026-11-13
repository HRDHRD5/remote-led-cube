
#ifndef LED_CUBE_CONTROLLER_WEBSERVER_H
#define LED_CUBE_CONTROLLER_WEBSERVER_H

#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Preferences.h>
#include "cubecontrol.h"

class WifiController
{
private:
    static Preferences prefs; // object for persistent storage
    String wifiSsid;
    String wifiPassword;
    void tryUntilWifiConnected();
    void tryUntilAPReady();
public:
    WifiController();
    void initWifi();
    void setWifi(String ssid, String password);
};

#endif
