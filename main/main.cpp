#include "../interfaces/CommonEngine.h"
#include "../networking/GameState.h"
#include "Player.h"
#include "Algo.h"
#include "LocalHuman.h"
#include "Computer.h"

#include <iostream>
#include <string>

const std::string algoLibName = "RandomAlgo";
const std::string engineLibName = "AtaxxEngine";

enum gameMode_t
{
	PVE,
	PVP,
	EVE
};

enum player
{
	PLAYER1,
	PLAYER2,

	NO_OF_PLAYERS
};

void printWelcome();
gameMode_t askForMode();
void setupPlayers( gameMode_t gameMode, Player ** player1, Player ** player2 );
player nextPlayer( player playerActive );

void printGameResult( engine::GameResult result ) 
{
	switch(result)
	{
	case engine::PLAYER_1:
		std::cout << "Player 1 won!\n";
		break;
	case engine::PLAYER_2:
		std::cout << "Player 2 won!\n";
		break;
	case engine::DRAW:
		std::cout << "It is a draw\n";
		break;
	}
	system("PAUSE");
}

void cleanUp(Player** players) 
{
	delete players[PLAYER1];
	delete players[PLAYER2];
}

int main()
{
	bool localDebug = true;
	printWelcome();
	gameMode_t gameMode = askForMode();
	Player * players[NO_OF_PLAYERS] = {NULL};//TODO: change to unique_ptrs
	setupPlayers(gameMode, &players[0], &players[1]);
	//TODO: thats an explicit call to uncompressed version of the function. and which one we use depends on algorithm, 
	// which is determined in runtime. this has to be solved fast
	// solution: export only PublicState, maybe make these call via algo
	net::GameState currentGameState = engine::getInitialState();
	
	
	while(engine::isGameFinished(currentGameState) == engine::NOT_FINISHED)
	{
		if(localDebug)
		{
			currentGameState.print();
		}
		currentGameState = players[currentGameState.getCurrentPlayerID()]->move(currentGameState);
	}
	printGameResult(engine::isGameFinished(currentGameState));
	cleanUp(players);
	return 0;
}

void printWelcome()
{
	//if anyone comes up with anything at least a tiny bit less dumb, you're welcome to replace it
	std::cout << "Welcome to KNAG's gaming app. Enjoy your attaxx/nmk!\n";
	std::cout << "This build supports nmk\n";
}

gameMode_t askForMode()
{
	std::cout << "Please choose a game mode. Available modes are PvP, PvE or EvE\n";
	std::string mode;
	for( ;; )
	{
		std::cin >> mode;
		if(mode == "PvP" || mode == "pvp")
		{
			return PVP;
		}
		else if(mode == "PvE" || mode == "pve")
		{
			return PVE;
		}
		else if (mode == "EvE" || mode == "eve")
		{
			return EVE;
		}
		else
		{
			std::cout << "Please enter a valid mode\n";
		}
	}
	
}

void setupPlayers( gameMode_t gameMode, Player ** player1, Player ** player2 ) 
{
	switch(gameMode)
	{
	case EVE:
		{
			*player1 = new Computer;
			*player2 = new Computer;
			break;
		}
	case PVE:
		{
			*player1 = new LocalHuman;
			*player2 = new Computer;
			break;
		}
	case PVP:
		{
			*player1 = new LocalHuman;
			*player2 = new LocalHuman;
			break;
		}
	}
}

player nextPlayer( player playerActive ) 
{
	if(playerActive == PLAYER1)
	{
		return PLAYER2;
	}
	else
	{
		return PLAYER1;
	}
}