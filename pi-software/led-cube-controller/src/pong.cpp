#include "../include/pong.h"

PongPlayer::PongPlayer()
    : id(-1)
{
}

PongPlayer::PongPlayer(byte _id, byte _boardX, byte _boardY)
    : id(_id), boardX(_boardX), boardY(_boardY), posX((CUBE_SIDE_LENGTH - 1) / 2), posY((CUBE_SIDE_LENGTH - 1) / 2)
{
}

void PongPlayer::move(bool moveX, bool movePositive)
{
    int move = movePositive ? 1 : -1;
    if (moveX)
        posX += move;
    else
        posY += move;
}

int PongPlayer::getId() const
{
    return id;
}

byte PongPlayer::getBoardX() const
{
    return boardX;
}

byte PongPlayer::getBoardY() const
{
    return boardY;
}

void PongGame::renderFrame()
{
    cube->clear();

    if (state == PONGGAMESTATE_WAITFORSTART)
    {
        for (int p = 0; p < pongPlayerCount; ++p)
        {
            switch (p)
            {
            case 0:
                cube->setPlane(cube->getNumberChar(p+1), 4, 0, 0, 4);
                break;
            case 1:
                cube->setPlane(cube->getNumberChar(p+1), 3, 7, 0, 1);
                break;
            case 2:
                cube->setPlane(cube->getNumberChar(p+1), 7, 3, 0, 2);
                break;
            case 3:
                cube->setPlane(cube->getNumberChar(p+1), 0, 4, 0, 5);
                break;
            case 4:
                cube->setPlane(cube->getNumberChar(p+1), 0, 3, 0, 3);
                break;
            case 5:
                cube->setPlane(cube->getNumberChar(p+1), 0, 3, 7, 3);
                break;
            default:
                cube->setPlane(cube->getNumberChar(p+1), 0, 3, 0, 3);
                break;
            }
        }
    }
}

void PongGame::checkBallPosition()
{
    
}

void PongGame::checkBallDirection()
{
    if (0 < ballX < CUBE_SIDE_LENGTH-1 &&
        0 < ballY < CUBE_SIDE_LENGTH-1 &&
        0 < ballZ < CUBE_SIDE_LENGTH-1 &&
        ballDirX != 0 && ballDirY != 0 && ballDirZ != 0)
    {
        return;
    }

    gameChanged = true;

    if (ballX == 0)
        ballDirX = 1;
    else if(ballX == CUBE_SIDE_LENGTH - 1)
        ballDirX = -1;
    else
        ballDirX = random(-1, 1);

    if (ballY == 0)
        ballDirY = 1;
    else if(ballY == CUBE_SIDE_LENGTH - 1)
        ballDirY = -1;
    else
        ballDirY = random(-1, 1);

    if (ballZ == 0)
        ballDirZ = 1;
    else if(ballZ == CUBE_SIDE_LENGTH - 1)
        ballDirZ = -1;
    else
        ballDirZ = random(-1, 1);
}

PongGame::PongGame(CubeController* _cube)
    : cube(_cube), ballSpeed(10), ballX((CUBE_SIDE_LENGTH-1)/2),
    ballY((CUBE_SIDE_LENGTH-1)/2), ballZ((CUBE_SIDE_LENGTH-1)/2),
    ballDirX(0), ballDirY(0), ballDirZ(0)
{
    if (!players)
        delete players;
    players = new PongPlayer[255];
    pongPlayerCount = 0;
}

PongGame::~PongGame()
{
    if (!players)
        delete players;

    cube->unregisterAnimationProvider();
    cube->setCubeMode(oldCubeMode);
}

PongGameState PongGame::getState()
{
    return state;
}

void PongGame::end(byte winnerId)
{
    delete this;
}

PongGame* PongGame::create(CubeController* cube)
{
    if (cube->animationRunning())
        return nullptr;

    PongGame* newGame = new PongGame(cube);
    newGame->oldCubeMode = cube->getCubeMode();
    cube->registerAnimationProvider(newGame);

    return newGame;
}

void PongGame::update()
{
    if (state == PONGGAMESTATE_WAITFORSTART)
    { }
    else if (state == PONGGAMESTATE_BALL_ALIVE)
    {
        checkBallDirection();
        checkBallPosition();
    }
    else if (PONGGAMESTATE_BALL_DEAD)
    { }
    else
    {
        state = PONGGAMESTATE_WAITFORSTART;
    }

    renderFrame();
}

const PongPlayer* const PongGame::playerJoin()
{
    if (state != PONGGAMESTATE_WAITFORSTART)
        return nullptr;

    if (pongPlayerCount >= 6)
        return nullptr;

    players[pongPlayerCount] = PongPlayer(pongPlayerCount, playerXBoardSize, playerYBoardSize);
    pongPlayerCount += 1;

    if (pongPlayerCount >= 1)
        state = PONGGAMESTATE_WAITFORSTART;

    return &(players[pongPlayerCount-1]);
}

void PongGame::movePlayer(byte id, bool moveX, bool movePositive)
{
    if (!ballDirX && !ballDirY && !ballDirZ)
        checkBallDirection();
}

PongPlayer* PongGame::getPlayer(byte id)
{
    for (int i = 0; i < pongPlayerCount; ++i)
    {
        if (players[i].getId() == id)
            return &players[i];
    }
    return nullptr;
}
