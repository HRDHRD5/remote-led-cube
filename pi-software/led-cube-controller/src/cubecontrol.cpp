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
    uint64_t time = (millisBaseTime + millis() - timeOffset) / 1000;
    hours = (time % 86400) / 3600;
    minutes = (time % 3600) / 60;
    seconds = time % 60;

    // Adjust for 12 Hour Format
    if (hours > 12 )
    {
        hours = hours - 12;
    }
    else if (hours == 0)
    {
        hours = 12;
    }
}

void CubeController::setCoord(byte x, byte y, byte z, bool on)
{
    if (on)
        clockFrame[(8*z)+y] |= (0x80 >> x);
    else
        clockFrame[(8*z)+y] &= (0x80 >> x ^ 0xFF);
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
                setCoord(x_cord, y_cord, z_cord, coordValue);
            }
        }
    }
}

void CubeController::setClockFrame()
{
    memset(clockFrame, 0, FRAME_LENGTH);
    // setting hour
    if (hours > 9)
        setPlane(charMap[hours / 10], 7, 7, 0, 5);
    setPlane(charMap[hours % 10], 7, 4, 0, 5);
    // setting minutes
    setPlane(charMap[minutes / 10], 7, 0, 0, 4);
    setPlane(charMap[minutes % 10], 3, 0, 0, 4);
    // setting seconds
    setPlane(charMap[seconds / 10], 0, 1, 0, 2);
    setPlane(charMap[seconds % 10], 0, 5, 0, 2);

    if (minutes == 0 && (hourAnimationCounter > 1 || seconds < 5))
    {
        if (hourAnimationCounter > 512)
        {
            hourAnimationCounter = 1;
        }
        else
        {
            // way to complecated spiral animation
            uint32_t inverseC = (512 - hourAnimationCounter);
            int x, y, dx, tmp;
            int dy = -1;
            byte drawn = 0;
            // full layers
            for (int z = 0; z < (inverseC / 64); ++z)
            {
                setPlane(planeOn, 0, 0, z, 0);
            }
            // top layer with spiral
            drawn = 0;
            x = 0;
            y = 0;
            dx = 0;
            tmp = 0;
            dy = -1;
            drawn = 0;
            for (int i = 0; i < 81 && drawn < inverseC % 64; ++i)
            {
                if (x+3 >= 0 && x+3 < 8 && y+3 >= 0 && y+3 < 8)
                {
                    ++drawn;
                    setCoord(x+3, y+3, (inverseC / 64), true);
                }

                if (x == y || (x < 0 && x == -y) || (x > 0 && x == 1-y))
                {
                    tmp = -dy;
                    dy = dx;
                    dx = tmp;
                }
                x += dx;
                y += dy;
                if (x > 10 || y > 10)
                {
                    break;
                }
            }
            ++hourAnimationCounter;
        }
    }
    else {
        // Animation every Minute
        if (seconds < 3)
        {
            if (minuteAnimationCounter < 16)
            {
                setCoord(3, 3, (minuteAnimationCounter/2), true);
                setCoord(3, 3, (minuteAnimationCounter/2)-1, true);
                setCoord(3, 4, (minuteAnimationCounter/2), true);
                setCoord(3, 4, (minuteAnimationCounter/2)-1, true);
                setCoord(4, 3, (minuteAnimationCounter/2), true);
                setCoord(4, 3, (minuteAnimationCounter/2)-1, true);
                setCoord(4, 4, (minuteAnimationCounter/2), true);
                setCoord(4, 4, (minuteAnimationCounter/2)-1, true);
            }
            else if (minuteAnimationCounter < 30)
            {
                byte layer = (32 - minuteAnimationCounter) / 2;
                // for loop defines the number of sparkles
                for (int i = 0; i < 15; ++i)
                {
                    setCoord(random(1,7), random(1,7), random(layer-3, layer), true);
                }
            }
            ++minuteAnimationCounter;
        }
        else
        {
            minuteAnimationCounter = 1;

            // Animation every 10 Seconds
            if (seconds % 10 == 0 && tenSecondAnimationCounter < 10)
            {
                ++tenSecondAnimationCounter;
                setPlane(charMap[seconds / 10], tenSecondAnimationCounter/2, 1, 0, 2);
                setPlane(charMap[seconds % 10], tenSecondAnimationCounter/2, 5, 0, 2);
            }
            else if (seconds % 10 != 0)
            {
                tenSecondAnimationCounter = 1;
            }
        }
    }

    // Seconds Animation Base Plane
    setPlane(secondsArrowMap[seconds % 4], 0, 0, 0, 0);
}

void CubeController::sendFrame(const char *frameHex, uint32_t frameLen)
{
    if (clockEnabled)
    {
        return;
    }

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

void CubeController::setBaseTime(uint64_t mills)
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