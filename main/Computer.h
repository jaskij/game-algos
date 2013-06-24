#pragma once
#include "player.h"
#include "Algo.h"
#include "AlgoFactory.h"
#include <string>

class Computer :
	public Player
{
public:
	Computer();
	~Computer(void);
	net::GameState move( net::GameState currentGameState );
private:
	algo::Algo* algorithm;
};

