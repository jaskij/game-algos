#include "Computer.h"


Computer::Computer()
{
	AlgoFactory factory;
	std::string algoName = "random";
	algorithm = factory.createAlgo(algoName);
}


Computer::~Computer(void)
{
}

engine::GameState* Computer::move( engine::GameState* currentGameState )
{
	return algorithm->chooseMove(currentGameState);
}

