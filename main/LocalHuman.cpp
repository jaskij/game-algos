#include "LocalHuman.h"
#include "utility.h"
#include <iostream>


LocalHuman::LocalHuman(void)
{
}


LocalHuman::~LocalHuman(void)
{
}

engine::GameState* LocalHuman::move( engine::GameState* currentGameState )
{
	bool moveCompleted = false;
	CoordU to;
	std::cout << "Where do you want to move?\n";
	while(!moveCompleted)
	{
		std::cin >> to.x >> to.y;
		//TODO: add to header, bool isMoveValid(GameState current, CoordU from, CoordU to);
		//Note, that this function will probably be unnecessary after implementing public state
		moveCompleted = engine::isMoveValid(currentGameState, CoordU(), to);
		if(!moveCompleted)
			std::cout << "It is not a valid move. Please enter another one\n";
	}
	return engine::makeMove(currentGameState, CoordU(), to);//note: same as above
}