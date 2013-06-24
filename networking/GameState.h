#pragma once
#include "../main/utility.h"
#ifdef _MSC_VER
#pragma warning(disable:100 510 610) //rationale: bogus warnings because of boost concept checking
#endif //_MSC_VER
#include <boost/multi_array.hpp>
#ifdef _MSC_VER
#pragma warning(default:100 510 610)
#endif // _MSC_VER
#include <boost/assign.hpp>
#include <string>
#include <map>

#ifdef _MSC_VER
	#ifdef DLL_BUILD
		#define DLL_API __declspec(dllexport)
	#else
		#define DLL_API __declspec(dllimport)
	#endif //DLL_BUILD
#else
	#define DLL_API
#endif // _MSC_VER


namespace net
{
	class DLL_API GameState
	{
		//all parameters have to be provided with constructor, setters only exist to reflect player input - 
		// maybe make them modify some temps(other object) and keep internals locked?
	public:
		typedef unsigned PlayerID;
		GameState(const unsigned boardW, const unsigned boardH, unsigned numPlayers, PlayerID startingPlayer = 0);
		void print();
		unsigned GameState::getBoardWidth()
		{
			return boardDimensions.x;
		}
		unsigned GameState::getBoardHeight()
		{
			return boardDimensions.y;
		}
		PlayerID getCurrentPlayerID()
		{
			return currentPlayer;
		}
		std::string getCurrentPlayerName()
		{
			return playerNames[currentPlayer];
		}
		void advanceActivePlayer()
		{
			if(++currentPlayer == playerNames.size())
				currentPlayer = 0;
		}
		void setPlayerName(PlayerID id, std::string name);
		bool isFieldEmpty(unsigned x, unsigned y)
		{
			return board[x][y] == EMPTY;
		}
		bool isFieldBlocked(unsigned x, unsigned y)
		{
			return board[x][y] == BLOCKED;
		}
		/// \return negative value is returned if field is not occupied by any player
		int getPlayerIDOnField(unsigned x, unsigned y)
		{
			return board[x][y] - PLAYER_1;
		}
		void setFieldEmpty(unsigned x, unsigned y)
		{
			board[x][y] = EMPTY;
		}
		void setFieldBlocked(unsigned x, unsigned y)
		{
			board[x][y] = BLOCKED;
		}
		void setPlayerIDOnField(unsigned x, unsigned y, PlayerID id)
		{
			assert(id < playerNames.size());
			board[x][y] = static_cast<fieldStates>(id + PLAYER_1);
		}
	private:
#ifdef _MSC_VER
#pragma warning(disable: 251) //rationale : private members don't need dll interface
#endif // _MSC_VER
		enum fieldStates
		{
			EMPTY = -2,
			BLOCKED = -1,
			PLAYER_1 = 0,
			PLAYER_2 = 1,
			PLAYER_3 = 2,
			PLAYER_4 = 3
		};
		void printBoard();

		CoordU boardDimensions;
		std::vector<std::string> playerNames;
		PlayerID currentPlayer;
		boost::multi_array<fieldStates, 2> board;
		static const std::map<fieldStates, char> printableFieldStates;
#ifdef _MSC_VER
#pragma warning(default: 251)
#endif // _MSC_VER
	};
}

