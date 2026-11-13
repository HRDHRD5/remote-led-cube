#include "../include/wifi.h"

Preferences WifiController::prefs{};

void WifiController::tryUntilWifiConnected()
{
    if (wifiSsid == "")
        return;
    if (wifiPassword == "")
        wifiPassword = "";

    WiFi.setHostname(WIFI_HOSTNAME);
    if (WiFi.status() == WL_IDLE_STATUS)
    {
        WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
    }

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
        Serial.println("Trying to connect to Wifi");
    }
    digitalWrite(LED_BUILTIN, HIGH);
}

void WifiController::tryUntilAPReady()
{
    WiFi.setHostname(WIFI_HOSTNAME);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.printf("Wifi status: %zu\n", WiFi.status());
        Serial.println("Trying to setup Access Point");
        WiFi.softAP(AP_WIFI_SSID, AP_WIFI_PASSWORD);
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
    }
}

void WifiController::initWifi()
{
    while (WiFi.status() != WL_IDLE_STATUS)
    {
        if (WiFi.softAPSSID() == AP_WIFI_SSID)
            WiFi.disconnectAP();
        else
            WiFi.disconnect();
    }

    // create own AP
    if (wifiSsid == "")
    {
        tryUntilAPReady();
    }
    else
    {
        tryUntilWifiConnected();
    }
    if (WiFi.softAPSSID() == AP_WIFI_SSID)
        Serial.println(WiFi.softAPIP());
    else
        Serial.println(WiFi.localIP());
}

void WifiController::setWifi(String ssid, String password)
{
    wifiSsid = ssid;
    wifiPassword = password;
    initWifi();
    prefs.putString("WIFI_SSID", ssid);
    prefs.putString("WIFI_PASSWORD", password);
}

WifiController::WifiController()
{
    wifiSsid = prefs.getString("WIFI_SSID", "");
    wifiPassword = prefs.getString("WIFI_PASSWORD", "");
}