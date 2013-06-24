#pragma once
#include "player.h"
#include "../interfaces/CommonEngine.h"

class LocalHuman :
	public Player
{
public:
	net::GameState move( net::GameState currentGameState );
};

