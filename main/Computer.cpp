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

net::GameState Computer::move( net::GameState currentGameState )
{
	return algorithm->chooseMove(currentGameState);
}

