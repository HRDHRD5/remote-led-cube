
#ifndef LED_CUBE_CONTROLLER_CUBECONTROL_H
#define LED_CUBE_CONTROLLER_CUBECONTROL_H

#include <stdint.h>
#include <Arduino.h>

#include "constants.h"

class CubeController
{
    private:
        uint32_t baudRate;

        void sendFrameUART(const byte frame[FRAME_LENGTH]);
    public:
        CubeController(uint32_t _baudRate);
        void sendFrame(const char *frameHex, uint32_t frameLen);
};
#endif