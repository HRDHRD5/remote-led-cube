
#ifndef LED_CUBE_CONTROLLER_WEBSERVER_H
#define LED_CUBE_CONTROLLER_WEBSERVER_H

#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "cubecontrol.h"

class WifiController
{
private:
    const char *wifiSsid;
    const char *wifiPassword;
public:
    WifiController(const char *_wifiSsid, const char *_wifiPassword);
    void tryUntilWifiConnected();
};

#endif
