#pragma once

#include <stdint.h>
#include "../main/utility.h"
#include "../interfaces/CommonEngine.h"

//rationale: typed enum is supported in other compilers
#ifdef _WIN32
#pragma warning(disable: 4480) 
#endif

/*!
\TODO make it possible to init isMoveValid, makeMove and evaluate same way as genMoves
*/

namespace engine
{
	const unsigned BOARD_WIDTH=7;
	const unsigned BOARD_HEIGHT=7;
	const unsigned BOARD_SIZE= BOARD_WIDTH * BOARD_HEIGHT;
	const unsigned COMPRESSED_SIZE = 13;

	/// \brief object describing a particular game state
	struct GameState
	{
		enum FieldState : uint8_t
		{
			EMPTY = 0,
			P_1 = 1,
			P_2 = 2,
			BLOCKED = 3
		};
		enum Player : uint8_t
		{
			ONE = 1,
			TWO = 2
		};

		FieldState board[BOARD_SIZE];
		Player player;
	};

	/// \brief a compressed GameState
	struct CompressedState
	{
		uint8_t data[COMPRESSED_SIZE];
	};

	/*! \brief compress a GameState
	\param State GameState to be compressed
	\param a CompressedState representing the same game state
	\return same as param2
	*/
	CompressedState* compress(const GameState* const State, CompressedState* out);

	/*! \brief decompress a CompressedState
	\param State GameState to be compressed
	\param CompressedState corresponding to first argument
	\return same as param2
	*/
	GameState* decompress(const CompressedState* const State, GameState* Out);

}