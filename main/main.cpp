#include "../interfaces/CommonEngine.h"
#include "../networking/GameState.h"
#include "Player.h"
#include "Algo.h"
#include "LocalHuman.h"
#include "Computer.h"

#include <iostream>
#include <string>

using std::vector;
using std::unique_ptr;

const std::string algoLibName = "RandomAlgo";
const std::string engineLibName = "AtaxxEngine";

enum gameMode_t
{
	PVE,
	PVP,
	EVE
};

void printWelcome();
gameMode_t askForMode();
vector<unique_ptr<Player>> setupPlayers( gameMode_t gameMode);

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

int main()
{
	bool localDebug = true;
	printWelcome();
	std::cout << std::endl << engine::getConfigString() << std::endl;
	gameMode_t gameMode = askForMode();
	//TODO: we should ask engine or net::GameOptions about num and types of players and make setup generic
	//probably net::GameOptions, engine shouldn't know who's playing
	auto Players = setupPlayers(gameMode);  
	net::GameState currentGameState = engine::getInitialState();
	
	while(engine::isGameFinished(currentGameState) == engine::NOT_FINISHED)
	{
		if(localDebug)
		{
			currentGameState.print();
		}
		currentGameState = Players[currentGameState.getCurrentPlayerID()]->move(currentGameState);
	}
	if (localDebug)
	{
		currentGameState.print();
	}
	printGameResult(engine::isGameFinished(currentGameState));
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

vector<unique_ptr<Player>> setupPlayers( gameMode_t gameMode )
{
	vector<unique_ptr<Player>> Players;
	switch(gameMode)
	{
	case EVE:
		{
			Players.emplace_back(new Computer);
			Players.emplace_back(new Computer);
			break;
		}
	case PVE:
		{
			Players.emplace_back(new LocalHuman);
			Players.emplace_back(new Computer);
			break;
		}
	case PVP:
		{
			Players.emplace_back(new LocalHuman);
			Players.emplace_back(new LocalHuman);
			break;
		}
	}
	return Players;
}
