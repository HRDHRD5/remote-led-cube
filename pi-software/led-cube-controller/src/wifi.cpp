#include "../include/wifi.h"

void WifiController::tryUntilWifiConnected()
{
    if (WiFi.status() == WL_IDLE_STATUS)
    {
        WiFi.begin(wifiSsid, wifiPassword);
    }

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        WiFi.begin(wifiSsid, wifiPassword);
        Serial.println("Trying to connect to Wifi");
    }
    digitalWrite(LED_BUILTIN, HIGH);
}

WifiController::WifiController(const char *_wifiSsid, const char *_wifiPassword)
{
    wifiSsid = _wifiSsid;
    wifiPassword = _wifiPassword;
}