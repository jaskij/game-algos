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

	net::GameState RandomAlgo::chooseMove( net::GameState publicState )
	{
		auto internalState = engine::netStateToUncomp(publicState);
		allocator.allocateStates(STATE_BUFFER_SIZE, stateBuffer);
		engine::initGen(internalState, stateBuffer, STATE_BUFFER_SIZE);
		unsigned movesGened = 0;
		unsigned noOfMoves = 0;
		do
		{
			movesGened = engine::genMovesUncomp();
			noOfMoves += movesGened;
			saveMoves();
			allocator.allocateStates(STATE_BUFFER_SIZE, stateBuffer);
		}
		while(movesGened == STATE_BUFFER_SIZE);
		assert(noOfMoves != 0);//if no moves were possible game should have been over
		unsigned moveIndex = rand() % noOfMoves;
		std::swap(internalState, *getStateAt(moveIndex));
		publicState = engine::convertToPublic(internalState);
		free(internalState);
		freeAllStates();
		return publicState;
	}

	engine::GameState** RandomAlgo::getStateAt( unsigned moveIndex )
	{
		unsigned vectorIndex = moveIndex / STATE_BUFFER_SIZE;
		unsigned arrayIndex = (moveIndex % STATE_BUFFER_SIZE);
		return &(possibleStateBuffers[vectorIndex])[arrayIndex];
	}

	void RandomAlgo::saveMoves()
	{
		engine::GameState** newBuffer = new engine::GameState* [STATE_BUFFER_SIZE];
		possibleStateBuffers.push_back(newBuffer);
		memcpy(possibleStateBuffers.back(), stateBuffer, sizeof(engine::GameState*) * STATE_BUFFER_SIZE);
	}

	engine::GameState* RandomAlgo::getMove(unsigned moveIndex)
	{
		unsigned vectorIndex = moveIndex / STATE_BUFFER_SIZE;
		unsigned arrayIndex = (moveIndex % STATE_BUFFER_SIZE);
		return (possibleStateBuffers[vectorIndex])[arrayIndex];
	}

	void RandomAlgo::freeAllStates()
	{
		for(auto i = possibleStateBuffers.begin(); i != possibleStateBuffers.end(); ++i)
		{
			allocator.freeStates(STATE_BUFFER_SIZE, *i);
			delete [] *i;
		}
		possibleStateBuffers.clear();
		if (*stateBuffer != NULL)
		{
			allocator.freeStates(STATE_BUFFER_SIZE, stateBuffer);
		}
	}


}
