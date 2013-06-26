#include "nmkEngine.h"
#include "../interfaces/CommonEngine.h"
#include "../main/utility.h"
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <algorithm>

namespace engine
{
	static GameState const *  current;
	static int currentField;
	static FieldState currentPlayer;
	static GameState newTemplate;

	void initGen(const GameState * const current)
	{
		engine::current = current;
		currentField = -1;
		
		currentPlayer = current->player == P_1 ? P1 : P2;
		newTemplate = *current;
		newTemplate.player = currentPlayer == P1 ? P_2 : P_1;
		
	}

	unsigned genMovesUncomp(GameState* buffer[], const uint32_t bufferSize)
	{
		for(unsigned i = 0; i < bufferSize; ++i)
		{
			memcpy(buffer[i], &newTemplate, sizeof(GameState));
		}

		unsigned noOfGeneratedMoves = 0;
		while (currentField < BOARD_SIZE && noOfGeneratedMoves < bufferSize)
		{
			++currentField;
			if(current->board[currentField]==EMPTY)
			{
				buffer[noOfGeneratedMoves]->board[currentField] = currentPlayer;
				++noOfGeneratedMoves;
			}
		}
		return noOfGeneratedMoves;
	}


	unsigned getGameStateSize()
	{
		return sizeof(GameState);
	}

	bool requiresOriginToMove()
	{
		return false;
	}



	GameState* makeMove(GameState* Current, CoordU /* 'from' doesn't make sense in this game */, CoordU to)
	{
		Current->at(to) = Current->player == P_1 ? P1 : P2;
		Current->player = Current->player == P_1 ? P_2 : P_1;
		return Current;
	}

	bool isCoordValid(CoordU coord)
	{
		return coord.x < BOARD_W && coord.y < BOARD_H;
	}

	bool isMoveValid(GameState *Current, CoordU /* 'from' doesn't make sense in this game */ , CoordU to)
	{
		return isCoordValid(to) &&Current->at(to)==EMPTY;
	}

	bool canMoveFrom( engine::GameState* , CoordU  )
	{
		//this function should never be called in this game
		assert(1==0);
		return false;//dummy
	}


	net::GameState getInitialState()
	{
		return net::GameState(BOARD_W, BOARD_H, NUM_PLAYERS);
	}

	net::GameState convertToPublic(const GameState* State)
	{
		net::GameState Public(BOARD_W, BOARD_H, 2, State->player - P1);
		for (int y = 0; y < BOARD_H; y++)
		{
			for (int x = 0; x < BOARD_W; x++)
			{
				if (State->at(x, y) == EMPTY)
				{
					Public.setFieldEmpty(x, y);
				} 
				else if (State->at(x, y) == BLOCKED)
				{
					Public.setFieldBlocked(x, y);
				} 
				else
				{
					Public.setPlayerIDOnField(x, y, State->at(x, y) - P1);
				}
			}
		}
		return Public;
	}

	struct sequence_t
	{
		FieldState player;
		unsigned lenght;

		sequence_t() : lenght(), player(EMPTY)
		{};

		inline void breakSequence(FieldState newPlayer)
		{
			lenght = 0;
			player = newPlayer;
		}
	};

	

	//finds if enough pieces are in a line to trigger a win for either player
	template <typename coordFunc_t>
	GameResult findSequence( const GameState* const State, const unsigned PRIMARY_DIMENSION, 
		const unsigned SECONDARY_DIMENSION, coordFunc_t coordFunc )
	{
		sequence_t sequence;
		FieldState current = EMPTY;
		FieldState previous = EMPTY;
		bool isBoardFull = true;

		for(unsigned i = 0; i < SECONDARY_DIMENSION; ++i)
		{
			for(unsigned i2 = 0; i2 < PRIMARY_DIMENSION; ++i2)
			{
				current = State->board[coordFunc(i,i2)];
				if(current == EMPTY) //can be reduced to isBoardFull = current;
					isBoardFull = false;
				if(previous != current)
					sequence.breakSequence(current);
				previous = current;
				++sequence.lenght;
				if(sequence.lenght == K && current != EMPTY)
					return sequence.player == P_1 ? PLAYER_1 : PLAYER_2;
			}
			sequence.lenght = 0;
		}
		return isBoardFull ? DRAW : NOT_FINISHED;
	}

	template <typename startPosCondition_t, typename diagonalXLimit_t>
	GameResult findSequenceDiagonal( const GameState* const State, Coord<int> start, startPosCondition_t startPosCondition, 
		const int startXChange, diagonalXLimit_t diagonalXLimit)
	{
		sequence_t sequence;
		FieldState current = EMPTY;
		FieldState previous = EMPTY;
		bool isBoardFull = true;
		assert(start.y-(int)1e8 < 0);//it fails if a) some idiot changed start to unsigned which breaks the if below b) something went very wrong with computing start
		if(start.y >= 0)//if there are any fitting diagonals(>=K)
		{
			//start is a starting coord for all diagonals with length >= K
			// iteration starts at the bottom of left(or right) column, goes up and then right(or left)
			for(; startPosCondition(start.x); start = start.y==0 ? Coord<int>(start.x + startXChange, 0) : Coord<int>(start.x, start.y-1))
			{
				unsigned diagonalLength = std::min(diagonalXLimit(start.x), BOARD_H - start.y);
				assert(diagonalLength >= K);
				for(unsigned i = 0; i < diagonalLength; ++i)
				{
					current = State->board[(start.y+i) * BOARD_W + (start.x+ startXChange * i)];
					if(current == EMPTY) //can be reduced to isBoardFull = current;
						isBoardFull = false;
					if(previous != current)
						sequence.breakSequence(current);
					previous = current;
					++sequence.lenght;
					if(sequence.lenght == K && current != EMPTY)
						return sequence.player == P_1 ? PLAYER_1 : PLAYER_2;
				}
				sequence.lenght = 0;
			}
		}
		return NOT_FINISHED;
	}

	inline bool isPlayer(GameResult state)
	{
		return state == P_1 || state == P_2;
	}

	GameResult isGameFinished(const net::GameState& State)
	{
		auto uncomp = netStateToUncomp(State);
		auto retVal = isGameFinished(uncomp);
		free(uncomp);
		return retVal;
	}

	GameResult isGameFinished(const GameState* const State)
	{
		//this is a very naive implementation, but optimizing without moving to 2 bit fields seems pointless

		GameResult vertical = findSequence(State, BOARD_H, BOARD_W, 
			[](unsigned i, unsigned i2){return i2 * BOARD_W + i;});
		if(isPlayer(vertical))
			return vertical;
		GameResult horizontal = findSequence(State, BOARD_W, BOARD_H, 
			[](unsigned i, unsigned i2){return i * BOARD_H + i2;});
		if( isPlayer(horizontal))
			return horizontal;

		GameResult downright = findSequenceDiagonal(State, Coord<int>(0, BOARD_H - K),
			[](unsigned x){return x <=  BOARD_W - K;}, 1, [] (unsigned x){return BOARD_W - x;});
		if(isPlayer(downright))
			return downright;
		GameResult downleft = findSequenceDiagonal(State, Coord<int>(BOARD_W-1, BOARD_H - K), 
			[](unsigned x){return x >=  K - 1;}, -1, [] (unsigned x){return x+1;});
		if(isPlayer(downleft))
			return downleft;

		return downleft;//each value could be used since they all check for empty fields
	}

	GameState* netStateToUncomp( net::GameState Public )
	{
		assert(Public.getBoardHeight() == BOARD_H);
		assert(Public.getBoardWidth() == BOARD_W);
		GameState* Uncomp = (GameState*)malloc(sizeof(GameState));
		Uncomp->player =  static_cast<Player>(Public.getCurrentPlayerID() + P_1);
		for (unsigned y = 0; y < BOARD_H; ++y)
		{
			for (unsigned x = 0; x < BOARD_W; ++x)
			{
				if (Public.isFieldEmpty(x, y))
				{
					Uncomp->at(x, y) = EMPTY;
				} 
				else if(Public.isFieldBlocked(x, y))
				{
					Uncomp->at(x, y) = BLOCKED;
				}
				else
				{
					assert(Public.getPlayerIDOnField(x, y) >= 0 && Public.getPlayerIDOnField(x, y) < 2);
					Uncomp->at(x, y) = static_cast<FieldState>(P1 + Public.getPlayerIDOnField(x, y));
				}
			}
		}
		return Uncomp;
	}

	std::string getConfigString()
	{
		std::stringstream sstream;
		sstream << "Board is " << BOARD_W << "x" << BOARD_H << ". First of " << NUM_PLAYERS << 
			" players to form a line of " << K << " his pieces wins.\n";
		return sstream.str();
	}
}

