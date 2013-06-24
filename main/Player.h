#pragma once

#include "../interfaces/CommonEngine.h"

class Player
{
public:
	virtual ~Player(void)
	{}
	virtual net::GameState move( net::GameState currentGameState ) = 0;
};


