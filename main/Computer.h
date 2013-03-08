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
	engine::GameState* move( engine::GameState* currentGameState );
private:
	algo::Algo* algorithm;
};

