#include "RandomAlgo.h"
#include <ctime>
#include <cstdlib>

namespace algo
{
	RandomAlgo::RandomAlgo()
	{
		srand((unsigned)time(NULL));
	}

	RandomAlgo::~RandomAlgo()
	{
		freeAllStates();
	}

	engine::GameState* RandomAlgo::chooseMove(const engine::GameState * const currentState)
	{
		freeAllStates();

		engine::initGen(currentState, stateBuffer, STATE_BUFFER_SIZE);
		unsigned movesGened = 0;
		unsigned noOfMoves = 0;
		do 
		{
			allocator.allocateStates(STATE_BUFFER_SIZE, stateBuffer);
			movesGened = engine::genMovesUncomp();
			noOfMoves += movesGened;
			saveMoves();
		} 
		while (movesGened == STATE_BUFFER_SIZE);
		unsigned moveIndex = rand() % noOfMoves;
		return getMove(moveIndex);
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
	}


}
