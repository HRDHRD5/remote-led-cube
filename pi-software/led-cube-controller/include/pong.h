
#ifndef LED_CUBE_CONTROLLER_PONGGAME_H
#define LED_CUBE_CONTROLLER_PONGGAME_H

#include <stdint.h>
#include <Arduino.h>

#include "constants.h"
#include "cubecontrol.h"

enum PongGameState
{
    PONGGAMESTATE_WAITFORSTART,
    PONGGAMESTATE_BALL_ALIVE,
    PONGGAMESTATE_BALL_DEAD
};

class PongPlayer
{
    private:
        int id;
        int posX, posY;
        byte boardX, boardY;
    public:
        PongPlayer();
        PongPlayer(byte _id, byte _boardX, byte _boardY);
        void move(bool moveX, bool movePositive);

        int getId() const;
        byte getBoardX() const;
        byte getBoardY() const;
};

class PongGame : AnimationProvider
{
    private:
        int ballX, ballY, ballZ;
        int8_t ballDirX, ballDirY, ballDirZ;
        PongGameState state = PONGGAMESTATE_WAITFORSTART;
        byte winnerPlayerId = 0;
        byte playerXBoardSize = 2;
        byte playerYBoardSize = 2;
        bool gameChanged = true;
        CubeMode oldCubeMode = CUBEMODE_OFF;
        uint ballSpeed;
        CubeController* cube;
        PongPlayer *players = nullptr;
        byte pongPlayerCount = 0;

        void renderFrame();
        void checkBallPosition();
        void checkBallDirection();
    public:
        PongGame(CubeController* cube);
        ~PongGame();
        PongGameState getState();
        void end(byte winnerId);
        static PongGame* create(CubeController* cube);
        void update();
        const PongPlayer* const playerJoin();
        void movePlayer(byte id, bool moveX, bool movePositive);
        PongPlayer* getPlayer(byte id);
};
#endif