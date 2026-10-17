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
}

void CubeController::setCoord(byte x, byte y, byte z, bool on)
{
    // Make sure coords are not out of rage:
    if (x > CUBE_SIDE_LENGTH)
        x = CUBE_SIDE_LENGTH-1;
    if (y > CUBE_SIDE_LENGTH)
        y = CUBE_SIDE_LENGTH-1;
    if (z > CUBE_SIDE_LENGTH)
        z = CUBE_SIDE_LENGTH-1;
    if (on)
        cubeFrame[(8*z)+y] |= (0x80 >> x);
    else
        cubeFrame[(8*z)+y] &= (0x80 >> x ^ 0xFF);
}

void CubeController::clear()
{
    memset(cubeFrame, 0, FRAME_LENGTH);
}

const byte* CubeController::getNumberChar(uint number)
{
    if (number > 9)
        return nullptr;

    return charMap[number];
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

void CubeController::drawLine(byte x1, byte y1, byte z1, byte x2, byte y2, byte z2)
{
    if (x1 > 7)
        x1 = 7;
    if (x2 > 7)
        x2 = 7;
    if (y2 > 7)
        y2 = 7;
    if (y2 > 7)
        y2 = 7;
    if (z2 > 7)
        z2 = 7;
    if (z2 > 7)
        z2 = 7;
    float posX = x1;
    float posY = y1;
    float posZ = z1;
    float deltaX, deltaY, deltaZ, deltaFacX, deltaFacY, deltaFacZ;
    float deltaFX = x1-x2;
    float deltaFY = y1-y2;
    float deltaFZ = z1-z2;

    deltaFacX = 1;
    deltaFacY = 1;
    deltaFacZ = 1;
    if (deltaFX > 0)
        deltaFacX = -1;
    if (deltaFY > 0)
        deltaFacY = -1;
    if (deltaFZ > 0)
        deltaFacZ = -1;

    if (abs(deltaFX) >= abs(deltaFY) && abs(deltaFX) >= abs(deltaFZ))
    {
        deltaX = 1.0;
        deltaY = deltaX * (deltaFY/deltaFX);
        deltaZ = deltaX * (deltaFZ/deltaFX);
    }
    else if (abs(deltaFY) >= abs(deltaFZ))
    {
        deltaY = 1.0;
        deltaX = deltaY * (deltaFX/deltaFY);
        deltaZ = deltaY * (deltaFZ/deltaFY);
    }
    else
    {
        deltaZ = 1.0;
        deltaX = deltaZ * (deltaFX/deltaFZ);
        deltaY = deltaZ * (deltaFY/deltaFZ);
    }

    deltaX *= deltaFacX;
    deltaY *= deltaFacY;
    deltaZ *= deltaFacZ;

    while (abs(posX-float(x1)) < abs(deltaFX) || abs(posY-float(y1)) < abs(deltaFY) || abs(posZ-float(z1)) < abs(deltaFZ))
    {
        setCoord(round(posX), round(posY), round(posZ), true);
        posX += deltaX;
        posY += deltaY;
        posZ += deltaZ;
    }
    setCoord(x2, y2, z2, true);
}

void CubeController::drawSphere(int x, int y, int z, int radius)
{
    radius = abs(radius);
    for (int layer = -((int)radius); layer <= radius; ++layer)
    {
        for (int planeRadius = -(radius - abs(layer)); planeRadius <= radius - abs(layer); ++planeRadius)
        {
            for (int linePosition = -(radius - abs(layer) - abs(planeRadius)); linePosition <= radius - abs(layer) - abs(planeRadius); ++linePosition)
            {
                setCoord(x+linePosition, y+planeRadius, z+layer, true);
            }
        }
    }
}

void CubeController::setClockFrameTwoSideClock()
{
    clear();
    if (hours > 9)
        setPlane(charMap[hours / 10], 0, 7, 0, 1);
    setPlane(charMap[hours % 10], 4, 7, 0, 1);
    // setting minutes
    setPlane(charMap[minutes / 10], 7, 6, 0, 5);
    setPlane(charMap[minutes % 10], 7, 2, 0, 5);

    for (int i = 0; i <= seconds; ++i)
    {
        byte j = (int(float(i)/4) % 8);
        if (i <= 7)
            drawLine(4, 4, 0, 7-j, 7, 0);
        else if (i <= 15)
            drawLine(3, 4, 0, 7-j, 7, 0);
        else if (i <= 22)
            drawLine(3, 4, 0, 0, 7-j, 0);
        else if (i <= 30)
            drawLine(3, 3, 0, 0, 7-j, 0);
        else if (i <= 37)
            drawLine(3, 3, 0, j, 0, 0);
        else if (i <= 45)
            drawLine(4, 3, 0, j, 0, 0);
        else if (i <= 52)
            drawLine(4, 3, 0, 7, j, 0);
        else if (i <= 59)
            drawLine(4, 3, 0, 7, j, 0);
    }
}

void CubeController::setClockFrameThreeSideClock()
{
    clear();

    // Adjust for 12 Hour Format
    if (hours > 12 )
    {
        hours = hours - 12;
    }
    else if (hours == 0)
    {
        hours = 12;
    }

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
    if (mode)
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

bool CubeController::setupCustomAnimation(uint maxFrames, uint custom_vars, void (CubeController::*caller)())
{
    if (!this->customAnimationVals)
    {
        randomSeed(millis());
        customAnimationVals = new uint32_t[3+custom_vars];
        customAnimationVals[0] = 0;
        customAnimationVals[1] = maxFrames;
        customAnimationVals[2] = mode;
        setCubeMode(CUBEMODE_ANIMATION);
        running_animation = caller;
        return true;
    }
    return false;
}

bool CubeController::endCustomAnimation()
{
    if (customAnimationVals[0] >= customAnimationVals[1])
    {
        setCubeMode(CubeMode(customAnimationVals[2]));
        running_animation = nullptr;
        delete customAnimationVals;
        customAnimationVals = nullptr;
        return true;
    }
    customAnimationVals[0] += 1;

    return false;
}

void CubeController::fireworksAnimation()
{
    clear();

    const int frameFactor = 3;
    #define EXPLOSION1 3
    #define EXPLOSION2 8
    #define EXPLOSION3 13
    if (setupCustomAnimation(100*frameFactor, 15, &CubeController::fireworksAnimation))
    {
        // different explosions
        customAnimationVals[EXPLOSION1] = 0;
        customAnimationVals[EXPLOSION2] = 0;
        customAnimationVals[EXPLOSION3] = 0;
    }

    // EXPLOSIONX == Frame Count
    // EXPLOSIONX+1 == X Coord
    // EXPLOSIONX+2 == Y Coord
    // EXPLOSIONX+3 == Max Height
    // EXPLOSIONX+4 == Explosion Radius

    if (customAnimationVals[EXPLOSION1] == 0 && (customAnimationVals[EXPLOSION3] > 7 || customAnimationVals[EXPLOSION3] == 0))
    {
        customAnimationVals[EXPLOSION1] = 1;
        customAnimationVals[EXPLOSION1+1] = random(1, 6);
        customAnimationVals[EXPLOSION1+2] = random(1, 6);
        customAnimationVals[EXPLOSION1+3] = random(2, 7);
        customAnimationVals[EXPLOSION1+4] = random(1, 3);
    }
    else if (customAnimationVals[EXPLOSION2] == 0 && customAnimationVals[EXPLOSION1] > 7)
    {
        customAnimationVals[EXPLOSION2] = 1;
        customAnimationVals[EXPLOSION2+1] = random(1, 6);
        customAnimationVals[EXPLOSION2+2] = random(1, 6);
        customAnimationVals[EXPLOSION2+3] = random(2, 7);
        customAnimationVals[EXPLOSION2+4] = random(1, 3);
    }
    else if(customAnimationVals[EXPLOSION3] == 0 && customAnimationVals[EXPLOSION2] > 7)
    {
        customAnimationVals[EXPLOSION3] = 1;
        customAnimationVals[EXPLOSION3+1] = random(1, 6);
        customAnimationVals[EXPLOSION3+2] = random(1, 6);
        customAnimationVals[EXPLOSION3+3] = random(2, 7);
        customAnimationVals[EXPLOSION3+4] = random(1, 3);
    }

    for (int i = EXPLOSION1; i <= EXPLOSION3; i += 5)
    {
        // make sure that first Frames of count frameFactor are skipped
        if (customAnimationVals[i] == 0)
        {
            continue;
        }
        if (customAnimationVals[i]/frameFactor < customAnimationVals[i+3])
        {
            setCoord(customAnimationVals[i+1], customAnimationVals[i+2], (customAnimationVals[i]/frameFactor), true);
        }
        else if ((customAnimationVals[i]/frameFactor) <= (customAnimationVals[i+3]+(customAnimationVals[i+4]*3)))
        {
            int currentRadius = ((customAnimationVals[i]/frameFactor) - customAnimationVals[i+3])/3;
            drawSphere(customAnimationVals[i+1], customAnimationVals[i+2], customAnimationVals[i+3], currentRadius);
        }
        else {
            customAnimationVals[i] = 0;
            continue;
        }

        customAnimationVals[i] += 1;
    }

    endCustomAnimation();
}

void CubeController::randomZigzagAnimation()
{
    clear();

    const int frameFactor = 1;
    #define ZIGZAG_COUNT 100
    #define ZIGZAG_POS_X 3
    #define ZIGZAG_POS_Y 4
    #define ZIGZAG_POS_Z 5

    if (setupCustomAnimation(150*frameFactor, 3*ZIGZAG_COUNT, &CubeController::randomZigzagAnimation))
    {
        // ZigZag Coordinats. Each is with an offset of 1
        for (int i = 0; i < ZIGZAG_COUNT; ++i)
        {
            customAnimationVals[ZIGZAG_POS_X+i] = random(0, 7);
            customAnimationVals[ZIGZAG_POS_Y+i] = random(0, 7);
            customAnimationVals[ZIGZAG_POS_Z+i] = random(0, 7);
        }
    }

    for (int z = 0; z < ZIGZAG_COUNT*3; z+=3)
    {
        if (customAnimationVals[0] % frameFactor == 0)
        {
            // 0 equals x pos, 1 equals x neg
            // 2 equals y pos, 3 equals y neg
            // 4 equals z pos, 5 equals z neg
            int direction = random(0,5);

            if (direction % 2 == 0)
                customAnimationVals[(ZIGZAG_POS_X+z) + (direction/2)]+=1;
            else
                customAnimationVals[(ZIGZAG_POS_X+z) + (direction/2)]-=1;

            // if postion moves out of bounds, just wrap around
            for (int i = 0; i < 3; ++i)
            {
                if (customAnimationVals[ZIGZAG_POS_X+z+i] < 1)
                    customAnimationVals[ZIGZAG_POS_X+z+i] = CUBE_SIDE_LENGTH;
                else if (customAnimationVals[ZIGZAG_POS_X+z+i] > CUBE_SIDE_LENGTH)
                    customAnimationVals[ZIGZAG_POS_X+z+i] = 1;
            }
        }

        setCoord(customAnimationVals[ZIGZAG_POS_X+z]-1, customAnimationVals[ZIGZAG_POS_Y+z]-1, customAnimationVals[ZIGZAG_POS_Z+z]-1, true);
    }

    endCustomAnimation();
}

void CubeController::snakeAnimation()
{
    clear();

    const int frameFactor = 8;
    #define SNAKE_LINKS_COUNT 4
    #define SNAKE_POS_X 3
    #define SNAKE_POS_Y 4
    #define SNAKE_POS_Z 5

    if (setupCustomAnimation(150*frameFactor, 3*SNAKE_LINKS_COUNT, &CubeController::snakeAnimation))
    {
        // ZigZag Coordinats. Each is with an offset of 1
        memset(customAnimationVals+SNAKE_POS_X, 4, 3*SNAKE_LINKS_COUNT);
    }

    if (customAnimationVals[0] % frameFactor == 0)
    {
        bool moved = false;
        while (!moved)
        {
            // shift coords to the next link in the snake
            for (int i = 3*(SNAKE_LINKS_COUNT-2); i >= 0; i-=3)
            {
                customAnimationVals[SNAKE_POS_X+i+3] = customAnimationVals[SNAKE_POS_X+i];
                customAnimationVals[SNAKE_POS_Y+i+3] = customAnimationVals[SNAKE_POS_Y+i];
                customAnimationVals[SNAKE_POS_Z+i+3] = customAnimationVals[SNAKE_POS_Z+i];
            }

            // 0 equals x pos, 1 equals x neg
            // 2 equals y pos, 3 equals y neg
            // 4 equals z pos, 5 equals z neg
            int direction = random(0,5);

            int newCoord;
            uint changingCoord = SNAKE_POS_X + (direction/2);
            if (direction % 2 == 0)
                newCoord = customAnimationVals[changingCoord] + 1;
            else
                newCoord = customAnimationVals[changingCoord] - 1;

            // check if Snake moves back into itself
            if (newCoord == customAnimationVals[changingCoord+3])
                continue; // repeat random direction process

            customAnimationVals[changingCoord] = newCoord;

            // if postion moves out of bounds, just wrap around
            for (int i = 0; i < 3; ++i)
            {
                if (customAnimationVals[SNAKE_POS_X+i] < 1)
                    customAnimationVals[SNAKE_POS_X+i] = CUBE_SIDE_LENGTH;
                else if (customAnimationVals[SNAKE_POS_X+i] > CUBE_SIDE_LENGTH)
                    customAnimationVals[SNAKE_POS_X+i] = 1;
            }

            moved = true;
        }
    }

    for (int link = 0; link < SNAKE_LINKS_COUNT*3; link+=3)
        setCoord(customAnimationVals[SNAKE_POS_X+link]-1, customAnimationVals[SNAKE_POS_Y+link]-1, customAnimationVals[SNAKE_POS_Z+link]-1, true);

    endCustomAnimation();
}

void CubeController::update()
{
    updateTime();
    if (mode == CUBEMODE_OFF)
    {
        clear();
        sendFrameUART(cubeFrame);
        return;
    }
    else if (mode == CUBEMODE_CLOCK)
    {
        setClockFrameThreeSideClock();
        sendFrameUART(cubeFrame);
    }
    else if (mode == CUBEMODE_ANIMATION)
    {
        if (running_animation)
        {
            (this->*CubeController::running_animation)();
            sendFrameUART(cubeFrame);
        }
        else
        {
            mode = CUBEMODE_OFF;
        }
    }
    else if (mode == CUBEMODE_ANIMATIONPROVIDER)
    {
        if (currentAnimationProv)
        {
            currentAnimationProv->update();
            sendFrameUART(cubeFrame);
        }
        else
        {
            mode = CUBEMODE_OFF;
        }
    }
}

bool CubeController::animationRunning()
{
    return running_animation != nullptr;
}

void CubeController::setBaseTime(uint64_t mills)
{
    timeOffset = millis();
    millisBaseTime = mills;
}

CubeMode CubeController::getCubeMode()
{
    return mode;
}

void CubeController::setCubeMode(CubeMode _mode)
{
    mode = _mode;
}

void CubeController::registerAnimationProvider(AnimationProvider* _animation)
{
    currentAnimationProv = _animation;
    setCubeMode(CUBEMODE_ANIMATIONPROVIDER);
}

void CubeController::unregisterAnimationProvider()
{
    if (currentAnimationProv)
    {
        currentAnimationProv = nullptr;
        mode = CUBEMODE_OFF;
    }
}

CubeController::CubeController(uint32_t _baudRate)
{
    baudRate = _baudRate;
    Serial1.begin(baudRate, SERIAL_8N1);
    pinMode(LED_BUILTIN, OUTPUT);
}
