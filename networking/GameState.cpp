#include "GameState.h"
#include <iostream>

using std::cout;
using std::endl;

namespace net
{
	GameState::GameState( const unsigned boardW, const unsigned boardH, unsigned numPlayers, PlayerID activePlayer /* = 0 */) : 
		boardDimensions(boardW, boardH), 
		board(boost::extents[boardH][boardW]),
		currentPlayer(activePlayer)
	{
		assert(activePlayer < numPlayers);
		for (unsigned i = 0; i < numPlayers; i++)
		{
			playerNames.push_back(std::to_string(i + 1));
		}
		std::fill(board.data(), board.data() + board.num_elements(), EMPTY);
	}

	void GameState::print()
	{
		cout << endl;
		cout << "Current player is "<< playerNames[currentPlayer] << endl;
		cout << endl;
		printBoard();
	}

	void GameState::printBoard()
	{
		for(unsigned y = 0; y < getBoardHeight(); ++y)
		{
			for(unsigned x = 0; x < getBoardWidth(); ++x)
			{
				cout << printableFieldStates.at(board[y][x]);
			}
			cout << endl;
		}	
		cout << endl;
	}

	void GameState::setPlayerName( PlayerID id, std::string name )
	{
		assert(id < playerNames.size());
		playerNames[id] = name;
	}
	const std::map<GameState::fieldStates, char> GameState::printableFieldStates = 
			boost::assign::map_list_of (EMPTY,'-') (BLOCKED,'x') (PLAYER_1,'1') (PLAYER_2,'2');
}
