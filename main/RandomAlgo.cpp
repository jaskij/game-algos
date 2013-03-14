#include "RandomAlgo.h"
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <cassert>

namespace algo
{
	RandomAlgo::RandomAlgo()
	{
#ifdef NDEBUG
		srand((unsigned)time(NULL));
#else
		srand(0);
#endif
		*stateBuffer = NULL;
	}

	RandomAlgo::~RandomAlgo()
	{
		freeAllStates();
	}

	engine::GameState* RandomAlgo::chooseMove( engine::GameState * currentState )
	{
		freeAllStates();

		allocator.allocateStates(STATE_BUFFER_SIZE, stateBuffer);
		engine::initGen(currentState, stateBuffer, STATE_BUFFER_SIZE);
		unsigned movesGened = 0;
		unsigned noOfMoves = 0;
		do 
		{
			movesGened = engine::genMovesUncomp();
			noOfMoves += movesGened;
			saveMoves();
			allocator.allocateStates(STATE_BUFFER_SIZE, stateBuffer);
		} 
		while (movesGened == STATE_BUFFER_SIZE);
		assert(noOfMoves != 0);//if no moves were possible game should have been over
		unsigned moveIndex = rand() % noOfMoves;
		std::swap(currentState, *getPtrToState(moveIndex));
		return currentState;
	}

	engine::GameState** RandomAlgo::getPtrToState(unsigned moveIndex)
	{
		unsigned vectorIndex = moveIndex / STATE_BUFFER_SIZE;
		unsigned arrayIndex = (moveIndex % STATE_BUFFER_SIZE);
		return &(possibleMoves[vectorIndex])[arrayIndex];
	}

	void RandomAlgo::saveMoves()
	{
		engine::GameState** newBuffer = new engine::GameState* [STATE_BUFFER_SIZE];
		possibleMoves.push_back(newBuffer);
		memcpy(possibleMoves.back(), stateBuffer, sizeof(engine::GameState*) * STATE_BUFFER_SIZE);
	}

	engine::GameState* RandomAlgo::getMove(unsigned moveIndex)
	{
		unsigned vectorIndex = moveIndex / STATE_BUFFER_SIZE;
		unsigned arrayIndex = (moveIndex % STATE_BUFFER_SIZE);
		return (possibleMoves[vectorIndex])[arrayIndex];
	}

	void RandomAlgo::freeAllStates()
	{
		for(auto i = possibleMoves.begin(); i != possibleMoves.end(); ++i)
		{
			allocator.freeStates(STATE_BUFFER_SIZE, *i);
			delete [] *i;
		}
		possibleMoves.clear();
		if (*stateBuffer != NULL)
		{
			allocator.freeStates(STATE_BUFFER_SIZE, stateBuffer);
		}
	}


}
