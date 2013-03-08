#pragma once
#include "player.h"
#include "../interfaces/CommonEngine.h"

class LocalHuman :
	public Player
{
public:
	LocalHuman(void);
	~LocalHuman(void);
	engine::GameState* move( engine::GameState* currentGameState );
};

