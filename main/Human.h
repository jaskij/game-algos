#pragma once
#include "player.h"
class Human :
	public Player
{
public:
	Human(void);
	~Human(void);
	net::GameState move( net::GameState currentGameState );
};

