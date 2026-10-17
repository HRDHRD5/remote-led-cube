
#ifndef LED_CUBE_CONTROLLER_CUBECONTROL_H
#define LED_CUBE_CONTROLLER_CUBECONTROL_H

#include <stdint.h>
#include <Arduino.h>

#include "constants.h"

enum CubeMode
{
    CUBEMODE_OFF,
    CUBEMODE_CLOCK,
    CUBEMODE_ANIMATION,
    CUBEMODE_ANIMATIONPROVIDER,
    CUBEMODE_REMOTE
};

class AnimationProvider
{
    public:
        virtual void update();
};

class CubeController
{
    private:
        uint32_t baudRate;
        uint64_t millisBaseTime;
        uint64_t timeOffset = 0;
        byte hours = 0;
        byte minutes = 0;
        byte seconds = 0;
        CubeMode mode = CUBEMODE_CLOCK;
        byte cubeFrame[FRAME_LENGTH];
        byte tenSecondAnimationCounter = 1;
        byte minuteAnimationCounter = 1;
        uint32_t hourAnimationCounter = 1;
        uint32_t *customAnimationVals = nullptr;
        void (CubeController::*running_animation)() = nullptr;
        AnimationProvider* currentAnimationProv = nullptr;
        const byte planeOn[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        const byte secondsArrowMap[4][8] = {
            {0x0,0x0,0x0,0x18,0x18,0x18,0x18,0x18},
            {0x0,0x0,0x0,0x1F,0x1F,0x0,0x0,0x0},
            {0x18,0x18,0x18,0x18,0x18,0x0,0x0,0x0},
            {0x0,0x0,0x0,0xF8,0xF8,0x0,0x0,0x0},
        };
        const byte charMap[10][8] = {
            /*0*/{0x1F,0x11,0x1F,0x0,0x0,0x0,0x0,0x0},
            /*1*/{0x2,0x1F,0x0,0x0,0x0,0x0,0x0,0x0},
            /*2*/{0x1D,0x15,0x17,0x0,0x0,0x0,0x0,0x0},
            /*3*/{0x15,0x15,0x1F,0x0,0x0,0x0,0x0,0x0},
            /*4*/{0x7,0x4,0x1F,0x0,0x0,0x0,0x0,0x0},
            /*5*/{0x17,0x15,0x1D,0x0,0x0,0x0,0x0,0x0},
            /*6*/{0x1F,0x15,0x1D,0x0,0x0,0x0,0x0,0x0},
            /*7*/{0x1,0x1,0x1F,0x0,0x0,0x0,0x0,0x0},
            /*8*/{0x1F,0x15,0x1F,0x0,0x0,0x0,0x0,0x0},
            /*9*/{0x7,0x5,0x1F,0x0,0x0,0x0,0x0,0x0}
        };

        void sendFrameUART(const byte frame[FRAME_LENGTH]);
        void updateTime();
        void setClockFrameTwoSideClock();
        void setClockFrameThreeSideClock();
    public:
        const byte* getNumberChar(uint number); // returns byte[8]
        void clear();
        void setPlane(const byte plane[8], const int x, const int y, const int z, byte orientation);
        void setCoord(byte x, byte y, byte z, bool on);
        void drawLine(byte x1, byte y1, byte z1, byte x2, byte y2, byte z2);
        void drawSphere(int x, int y, int z, int radius);
        CubeController(uint32_t _baudRate);
        void sendFrame(const char *frameHex, uint32_t frameLen);
        void update();
        bool animationRunning();
        void setBaseTime(uint64_t mills);
        CubeMode getCubeMode();
        void setCubeMode(CubeMode _mode);
        void registerAnimationProvider(AnimationProvider* _animation);
        void unregisterAnimationProvider();
        bool setupCustomAnimation(uint maxFrames, uint custom_vars, void (CubeController::*caller)());
        bool endCustomAnimation();

        // Custom Animations
        void fireworksAnimation();
        void randomZigzagAnimation();
        void snakeAnimation();
};
#endif