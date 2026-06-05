#include "../include/cubecontrol.h"

byte hexToByte(const char number[2])
{
    byte result = 0;
    if (number[0] > 47 && number[0] < 58)
    {
        result += number[0] - 48;
    } else if (number[0] > 64 && number[0] < 71)
    {
        result += number[0] - 55;
    }

    if (number[1] > 47 && number[1] < 58)
    {
        result += (number[1] - 48) << 4;
    } else if (number[1] > 64 && number[1] < 71)
    {
        result += (number[1] - 55) << 4;
    }

    return result;
}

void CubeController::sendFrameUART(const byte frame[FRAME_LENGTH])
{
    Serial1.write(0xf2); // batch update supported
    for (byte i = 0; i < FRAME_LENGTH; i++)
    {
        Serial1.write(frame[i]);
    }
    // minimum update latency
    delay(20);
}

void CubeController::sendFrame(const char *frameHex, uint32_t frameLen)
{
    digitalWrite(LED_BUILTIN, LOW);
    byte frame[FRAME_LENGTH];

    Serial.println("Trying to send Frame:");
    Serial.println(frameHex);

    for (int i = 0; i < FRAME_LENGTH; ++i)
    {
        if (i < frameLen-1)
        {
            frame[i] = hexToByte(frameHex + (2 * i));
            Serial.printf("%02x", frame[i]);
        }
        else {
            frame[i] = 0x00;
        }
    }
    Serial.println();

    sendFrameUART(frame);
    digitalWrite(LED_BUILTIN, HIGH);
}

CubeController::CubeController(uint32_t _baudRate)
{
    baudRate = _baudRate;
    Serial1.begin(baudRate, SERIAL_8N1);
    pinMode(LED_BUILTIN, OUTPUT);
}