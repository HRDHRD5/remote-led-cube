#include "../include/cubecontrol.h"

byte hexToByte(const char number[2])
{
    byte result = 0;
    if (number[0] > 47 && number[0] < 58)
    {
        result += number[0] - 48 << 4;
    } else if (number[0] > 64 && number[0] < 71)
    {
        result += number[0] - 55 << 4;
    }

    if (number[1] > 47 && number[1] < 58)
    {
        result += (number[1] - 48);
    } else if (number[1] > 64 && number[1] < 71)
    {
        result += (number[1] - 55);
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

void CubeController::updateTime()
{
    long time = (millisBaseTime + millis() - timeOffset) / 1000;
    hours = (time % 8640) / 3600;
    minutes = (time % 3600) / 60;
    seconds = time % 60;
}

void CubeController::setPlane(const byte plane[8], const int x, const int y, const int z, byte orientation)
{
    /* Orientation:
        0 = XY
        1 = XZ
        2 = YZ
        3 = XY reverse
        4 = XZ reverse
        5 = YZ reverse
        6 = XY reverse and inverted
        7 = XZ reverse and inverted
        8 = YZ reverse and inverted
    */
    int x_cord = 0;
    int y_cord = 0;
    int z_cord = 0;
    int a_cord = 0;
    int b_cord = 0;
    for (int a = 0; a < 8; a++)
    {
        a_cord = a;
        if (orientation > 2)
            a_cord = -a_cord;
        for (int b = 0; b < 8; b++)
        {
            b_cord = b;
            if (orientation > 5)
                b_cord = -b_cord;
            if (orientation == 0 || orientation == 3 || orientation == 6)
            {
                x_cord = x+a_cord;
                y_cord = y+b_cord;
                z_cord = z;
            }
            else if (orientation == 1 || orientation == 4 || orientation == 7)
            {
                x_cord = x+a_cord;
                y_cord = y;
                z_cord = z+b_cord;
            }
            else
            {
                x_cord = x;
                y_cord = y+a_cord;
                z_cord = z+b_cord;
            }

            bool coordValue = false;
            if ((plane[a] & 0x80 >> b) > 0)
                coordValue = true;

            if (x_cord >= 0 && x_cord < 8 &&
                y_cord >= 0 && y_cord < 8 &&
                z_cord >= 0 && z_cord < 8)
            {
                if (coordValue)
                    clockFrame[(8*z_cord)+y_cord] = (clockFrame[(8*z_cord)+y_cord] | (0x80 >> x_cord));
                else
                    clockFrame[(8*z_cord)+y_cord] = (clockFrame[(8*z_cord)+y_cord] & (0x80 >> x_cord ^ 0xFF));
            }
        }
    }
}

void CubeController::setClockFrame()
{
    memset(clockFrame, 0, FRAME_LENGTH);
    // setting hour
    if (hours > 9)
        setPlane(charMap[hours / 10], 7, 0, 0, 5);
    setPlane(charMap[hours % 10], 7, 4, 0, 5);
    // setting minutes
    setPlane(charMap[minutes / 10], 7, 0, 0, 4);
    setPlane(charMap[minutes % 10], 3, 0, 0, 4);
    // setting seconds
    setPlane(charMap[seconds / 10], 0, 1, 0, 2);
    setPlane(charMap[seconds % 10], 0, 5, 0, 2);
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

void CubeController::update()
{
    if (clockEnabled)
    {
        updateTime();
        setClockFrame();
        sendFrameUART(clockFrame);
    }
}

void CubeController::setBaseTime(long mills)
{
    timeOffset = millis();
    millisBaseTime = mills;
}

void CubeController::setClockEnabled(bool enabled)
{
    clockEnabled = enabled;
}

CubeController::CubeController(uint32_t _baudRate)
{
    baudRate = _baudRate;
    Serial1.begin(baudRate, SERIAL_8N1);
    pinMode(LED_BUILTIN, OUTPUT);
}