#include <Arduino.h>

#include "../include/constants.h"
#include "../include/wifi.h"
#include "../include/cubecontrol.h"
#include "../include/pong.h"

CubeController cController(BAUDRATE);
PongGame* currentPongGame = nullptr;
WifiController wifiController(WIFI_SSID, WIFI_PASSWORD);
WebServer websrv(80);

void handleRoot()
{
    websrv.send(200, "text/html", WEBSITE_HTML);
}

void setClockTime()
{
    String clockSet = websrv.arg("plain");
    cController.setBaseTime(strtoull(clockSet.c_str(), nullptr, 10));

    websrv.send(200, "text/plain", "New Time Set");
}

void handleOneTimeAnimation()
{
    String animation = websrv.arg("plain");
    if (animation == "fireworks")
    {
        cController.fireworksAnimation();
    }
    else if (animation == "zigzag")
    {
        cController.randomZigzagAnimation();
    }
    else if (animation == "snake")
    {
        cController.snakeAnimation();
    }
    else
    {
        websrv.send(403, "text/plain", "There is no such animation");
        return;
    }

    websrv.send(200, "text/plain", "Started Animation");
}

void setCubeMode()
{
    String clockSet = websrv.arg("plain");
    clockSet.toLowerCase();

    if (currentPongGame && currentPongGame->getState() != PONGGAMESTATE_WAITFORSTART)
    {
        websrv.send(403, "text/plain", "Pong Game running");
    }
    else {
        if (currentPongGame)
        {
            currentPongGame->end(0);
            currentPongGame = nullptr;
        }
    }

    if (clockSet == "clock")
        cController.setCubeMode(CUBEMODE_CLOCK);
    else if (clockSet == "pong")
    {
        currentPongGame = PongGame::create(&cController);
    }
    else if (clockSet == "remote")
        cController.setCubeMode(CUBEMODE_REMOTE);
    else
    {
        cController.setCubeMode(CUBEMODE_OFF);
    }

    websrv.send(200, "text/plain", "Mode updated");
}

void getCubeMode()
{
    String mode = "off";

    if (cController.getCubeMode() == CUBEMODE_CLOCK ||
        cController.getCubeMode() == CUBEMODE_ANIMATION)
        mode = "clock";
    else if (cController.getCubeMode() == CUBEMODE_REMOTE)
        mode = "remote";
    else if (cController.getCubeMode() == CUBEMODE_ANIMATIONPROVIDER)
        mode = "pong";

    websrv.send(200, "text/plain", mode);
}

void pongJoinGame()
{
    if (!currentPongGame)
    {
        websrv.send(404, "text/plain", "There is currently no Pong Game running");
        return;
    }

    const PongPlayer* newPlayer = currentPongGame->playerJoin();

    if (!newPlayer)
    {
        websrv.send(403, "text/plain", "The game is already full");
        return;
    }

    websrv.send(200, "text/plain", String(newPlayer->getId()));
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
    websrv.on("/cube/setmode", HTTP_POST, setCubeMode);
    websrv.on("/cube/getmode", HTTP_GET, getCubeMode);
    websrv.on("/pong/join", HTTP_POST, pongJoinGame);
    websrv.on("/animation/onetime", HTTP_POST, handleOneTimeAnimation);
    websrv.onNotFound(handleNotFound);
    websrv.begin();
}

void setup()
{
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