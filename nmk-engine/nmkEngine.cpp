#include "nmkEngine.h"
#include "../interfaces/CommonEngine.h"
#include "../main/utility.h"
#include <cassert>
#include <cstring>
#include <cstdlib>

namespace engine
{
	static GameState const *  current;
	static GameState ** buffer;
	static uint32_t bufferSize;
	static int currentField;
	static FieldState currentPlayer;
	static GameState newTemplate;

	void initGen(const GameState * const current, GameState* buffer[], const uint32_t bufferSize)
	{
		engine::current = current;
		engine::buffer = buffer;
		engine::bufferSize = bufferSize;
		currentField = -1;
		
		currentPlayer = current->player == P_1 ? P1 : P2;
		newTemplate = *current;
		newTemplate.player = currentPlayer == P1 ? P_2 : P_1;
		
	}

	unsigned genMovesUncomp()
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


	//rationale: parameters are not needed and functions are temporary anyway
	#ifdef _MSC_VER
	#pragma warning(disable: 4100) 
	#endif
	GameState* makeMove(GameState* Current, CoordU from, CoordU to)
	{
		Current->at(to) = Current->player == P_1 ? P1 : P2;
		Current->player = Current->player == P_1 ? P_2 : P_1;
		return Current;
	}

	bool isMoveValid(GameState *Current, CoordU from, CoordU to)
	{
		return Current->at(to)==EMPTY;
	}

	bool canMoveFrom( engine::GameState* currentGameState, CoordU from )
	{
		//this function should never be called in this game
		assert(1==0);
		return false;//dummy
	}
#ifdef _MSC_VER
#pragma warning(default: 4100) 
#endif

	GameState* getInitialStateUncomp()
	{
		GameState* initialState = (GameState*)malloc(sizeof (GameState));
		memset(initialState, 0, sizeof(GameState));
		initialState->player = P_1;
		return initialState;
	}

	PublicState convertToPublic(GameState* State)
	{
		PublicState Public;
		memcpy(Public.board, State->board, sizeof(FieldState) * BOARD_SIZE);
		Public.player = State->player;
		return Public;
	}

	GameResult isGameFinished(const GameState* const State)
	{
		//this is a very naive implementation, but optimizing without moving to 2 bit fields seems pointless
		unsigned p1 = 0, p2 = 0, empty = 0;
		for(unsigned i = 0; i < BOARD_H; ++i)
		{
			for(unsigned i2 = 0; i2 < BOARD_W; ++i2)
			{
				switch(State->board[i * BOARD_W + i2])
				{
				case EMPTY: ++empty; break;
				case P_1: ++p1; break;
				case P_2: ++p2; break;
				}
				if(p1 == K)
					return PLAYER_1;
				if(p2 == K)
					return PLAYER_2;
			}
			p1=p2=0;
		}
		for(unsigned i = 0; i < BOARD_W; ++i)
		{
			for(unsigned i2 = 0; i2 < BOARD_H; ++i2)
			{
				switch(State->board[i2 * BOARD_H + i])
				{
				case EMPTY: ++empty; break;
				case P_1: ++p1; break;
				case P_2: ++p2; break;
				}
				if(p1 == K)
					return PLAYER_1;
				if(p2 == K)
					return PLAYER_2;
			}
			p1=p2=0;
		}
		if (empty > 0)
		{
			return NOT_FINISHED;
		} 
		else
		{
			return DRAW;
		}
	}
}

