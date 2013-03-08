#pragma once
#include "../interfaces/CommonEngine.h"

namespace algo
{
	class Algo
	{
	public:
		virtual ~Algo(void)
		{}

		/// \brief chooses one of the moves available at current game state
		/// \return returned object is guaranteed to exist only until next call non-const method of this class. 
		/// if you intend to preserve state history either convert them to public and keep in that form or copy  
		virtual engine::GameState* chooseMove(const engine::GameState * const currentState) = 0;
	};

}
