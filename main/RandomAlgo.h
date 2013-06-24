#pragma once
#include "Algo.h"
#include "../interfaces/CommonEngine.h"
#include <vector>
#include "MemoryAllocator.h"

namespace algo
{
	class RandomAlgo
		: public Algo
	{
	public:
		RandomAlgo();
		~RandomAlgo();

		/// \brief chooses one of the moves available at current game state
		/// \return returned object is guaranteed to exist only until next call non-const method of this class. 
		/// if you intend to preserve state history either convert them to public and keep in that form or copy  
		net::GameState chooseMove( net::GameState  currentState);

		
	private:

		void saveMoves();
		engine::GameState* getMove(unsigned moveIndex);
		void freeAllStates();
		engine::GameState** RandomAlgo::getStateAt(unsigned index);
		static const uint32_t STATE_BUFFER_SIZE = 100;
		engine::GameState* stateBuffer[STATE_BUFFER_SIZE];
		std::vector<engine::GameState**> possibleStateBuffers;
		MemoryAllocator allocator;
	};
}
