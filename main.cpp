// COMP710 - KineticImpact
// main.cpp - Entry point

#include "Game.h"

int main(int argc, char* argv[])
{
    Game game;

    if (!game.initialise())
        return 1;

    game.run();

    return 0;
}
