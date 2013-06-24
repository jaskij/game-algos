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
		virtual net::GameState chooseMove( net::GameState  currentState) = 0;
	};
}
