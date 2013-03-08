#pragma once
#include "player.h"
class Human :
	public Player
{
public:
	Human(void);
	~Human(void);
	engine::GameState* move( engine::GameState* currentGameState );
};

