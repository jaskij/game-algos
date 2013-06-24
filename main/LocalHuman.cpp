#include "LocalHuman.h"
#include "utility.h"
#include <iostream>

net::GameState LocalHuman::move( net::GameState currentGameState )
{
	bool moveCompleted = false, pieceChosen = false;
	auto internalState = engine::netStateToUncomp(currentGameState);
	CoordU from, to;
	if(engine::requiresOriginToMove())
	{
		std::cout << "Which piece would you like to move?\n";
		while(!pieceChosen)
		{
			std::cin >> from.x >> from.y;
			pieceChosen = engine::canMoveFrom(internalState, from);
			if(!pieceChosen || !std::cin.good())
			{
				if(!std::cin.good())
				{
					std::cin.sync();
					std::cin.clear();
					pieceChosen = false;
				}
				std::cout << "You don't have a piece at these coordinates. Try again\n";
			}
		}
	}
	std::cout << "Where do you want to move?\n";
	while(!moveCompleted)
	{
		std::cin >> to.x >> to.y;
		//Note, that this function will probably be unnecessary after implementing public state
		moveCompleted = engine::isMoveValid(internalState, from, to);
		if(!moveCompleted || !std::cin.good())
		{
			if(!std::cin.good())
			{
				std::cin.sync();
				std::cin.clear();
				moveCompleted = false;
			}
			std::cout << "It is not a valid move. Please enter another one\n";
		}
	}
	auto newState = engine::makeMove(internalState, from, to);//note: same as above
	auto retVal = engine::convertToPublic(newState);
	free(newState);
	return retVal;
}