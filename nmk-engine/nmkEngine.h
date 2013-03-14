#include "../interfaces/CommonEngine.h"

//rationale: typed enum is supported in other compilers
#ifdef _MSC_VER
#pragma warning(disable: 4480) 
#endif

namespace engine
{
	//for now set to 7 to match PublicStatefrom CommonEngine, later will be set in a separate header
	const unsigned BOARD_W = 7;
	const unsigned BOARD_H = 7;
	const int BOARD_SIZE = BOARD_H * BOARD_W;
	const unsigned K = 7;

	enum FieldState : uint8_t
	{
		EMPTY = 0,
		P1 = 1,
		P2 = 2,
		BLOCKED = 3
	};
	enum Player : uint8_t
	{
		P_1 = P1,
		P_2 = P2	
	};

	struct GameState
	{
		FieldState board[BOARD_SIZE];
		Player player;

		FieldState & at(unsigned row, unsigned col)
		{
			return board[row * BOARD_W + col];
		}

		FieldState & at(CoordU c)
		{
			return board[c.y * BOARD_W + c.x];
		}
	};
	//current implementation uses byte sized fields, but engine is bound to perform better with fields compressed to 2
	// bits, both due to cache and better implementations of some functions

	//possible optimization: if blocked fields cannot be added during gameplay, then keep separate state witch blocked 
	// fields replaced with empty ones, so checking for long enough sequence is trivial

}
	

	

