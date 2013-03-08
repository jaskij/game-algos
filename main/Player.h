#pragma once

#include "../interfaces/CommonEngine.h"

class Player
{
public:
	virtual ~Player(void)
	{}
	virtual engine::GameState* move( engine::GameState* currentGameState ) = 0;
};


