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

	bool isCoordValid(CoordU coord)
	{
		return coord.x < BOARD_W && coord.y < BOARD_H;
	}

	bool isMoveValid(GameState *Current, CoordU from, CoordU to)
	{
		return isCoordValid(to) &&Current->at(to)==EMPTY;
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


	static const unsigned victoryDecidingFieldStates = NO_OF_PLAYERS + 1;//1 is for EMPTY field state
	enum direction
	{
		HORIZONTAL,
		VERTICAL
	};

	//finds if enough pieces are in a line to trigger a win for either player
	GameResult findSequence( const GameState* const State, unsigned counts[victoryDecidingFieldStates], 
		const unsigned PRIMARY_DIMENSION, const unsigned SECONDARY_DIMENSION, direction DIRECTION )
	{
		FieldState current = EMPTY;
		FieldState currentSequence = EMPTY;
		for(unsigned i = 0; i < SECONDARY_DIMENSION; ++i)
		{
			for(unsigned i2 = 0; i2 < PRIMARY_DIMENSION; ++i2)
			{
				switch(DIRECTION)
				{
				case VERTICAL:
					current = State->board[i2 * BOARD_W + i];
					break;
				case HORIZONTAL:
					current = State->board[i * BOARD_H + i2];
					break;
				}
				counts[current]++;
				if(currentSequence != current && currentSequence != EMPTY)
					counts[currentSequence] = 0;
				currentSequence = current;
				if(counts[P_1] == K)
					return PLAYER_1;
				if(counts[P_2] == K)
					return PLAYER_2;
			}
			counts[P_1] = counts[P_2] = 0;
		}
		return NOT_FINISHED;
	}

	GameResult isGameFinished(const GameState* const State)
	{
		//this is a very naive implementation, but optimizing without moving to 2 bit fields seems pointless
		// TODO: get rid of code duplication without losing performance(easier said then done). 
		// also consider rotating the board instead of writing multiple functions
		// NOTE: empty fields are being calculated multiple times, but since their value is only checked against zero 
		// it isn't a problem. look into it in a future, optimized implementation
		unsigned counts[victoryDecidingFieldStates] = {0};
		
		GameResult vertical = findSequence(State, counts, BOARD_H, BOARD_W, VERTICAL);
		if(vertical != NOT_FINISHED)
			return vertical;
		GameResult horizontal = findSequence(State, counts, BOARD_W, BOARD_H, HORIZONTAL);
		if(horizontal != NOT_FINISHED)
			return horizontal;

		FieldState current = EMPTY;
		FieldState currentSequence = EMPTY;
		CoordU start(0, BOARD_H - K);

		if(start.y >= 0)//if there are any fitting diagonals(>=K)
		{
			//start is a starting coord for all diagonals with length >= K
			// iteration starts at the bottom of left column, goes up and then right
			for(; start.x <=  BOARD_W - K; start = start.y==0 ? CoordU(start.x+1, 0) : CoordU(start.x, start.y-1))
			{
				unsigned diagonalLength = std::min(BOARD_W - start.x, BOARD_H - start.y);
				assert(diagonalLength >= K);
				for(unsigned i = 0; i < diagonalLength; ++i)
				{
					current = State->board[(start.y+i) * BOARD_W + (start.x+i)];
					counts[current]++;
					if(currentSequence != current && currentSequence != EMPTY)
						counts[currentSequence] = 0;
					currentSequence = current;
					if(counts[P_1] == K)
						return PLAYER_1;
					if(counts[P_2] == K)
						return PLAYER_2;
				}
				counts[P_1] = counts[P_2] = 0;
			}
		}

		current = EMPTY;
		currentSequence = EMPTY;
		start = CoordU(BOARD_W-1, BOARD_H - K);

		if(start.y >= 0)//if there are any fitting diagonals(>=K)
		{
			//similar to previous loop, but starts in right column and searches downleft
			for(; start.x >= K - 1; start = start.y==0 ? CoordU(start.x-1, 0) : CoordU(start.x, start.y-1))
			{
				unsigned diagonalLength = std::min(start.x + 1, BOARD_H - start.y);
				assert(diagonalLength >= K);
				for(unsigned i = 0; i < diagonalLength; ++i)
				{
					current = State->board[(start.y+i) * BOARD_W + (start.x-i)];
					counts[current]++;
					if(currentSequence != current && currentSequence != EMPTY)
						counts[currentSequence] = 0;
					currentSequence = current;
					if(counts[P_1] == K)
						return PLAYER_1;
					if(counts[P_2] == K)
						return PLAYER_2;
				}
				counts[P_1] = counts[P_2] = 0;
			}
		}

		if (counts[EMPTY] > 0)
		{
			return NOT_FINISHED;
		} 
		else
		{
			return DRAW;
		}
	}
}

