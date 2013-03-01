#pragma once

#include <stdint.h>

/*! 
	\TODO	make it possible to init isMoveValid, makeMove and evaluate same way as genMoves
*/

namespace engine
{
	const unsigned BOARD_SIZE=49;
	const unsigned BOARD_WIDTH=7;
	const unsigned BOARD_HEIGHT=7;
	const unsigned COMPRESSED_SIZE = 13;

	struct CoordU
	{
		unsigned x,y;
		CoordU(unsigned x=0, unsigned y=0) : x(x), y(x) {};
	};

	/// \brief object describing a particular game state
	struct GameState{
		enum FieldState : uint8_t{
			EMPTY = 0,
			P_1 = 1,
			P_2 = 2,
			BLOCKED = 3
		};
		enum Player : uint8_t{
			ONE = 1,
			TWO = 2
		};

		FieldState board[BOARD_SIZE];
		Player player;
	};

	///	\brief a compressed GameState
	struct CompressedState{
		uint8_t data[COMPRESSED_SIZE];
	};

	/*! \brief	for use with server communications and GUI
		Should be redesigned to make it generic(fitting all games)
	*/
	struct PublicState
	{
		/// \brief	whose turn it is
		uint8_t player;
		/// \brief	state of the whole board
		uint8_t board[49];
	};

	/// \brief converts game state to verbose version, designed to be easily interpreted outside of the engine. 
	/// see PublicState definition for details
	PublicState convertToPublic(const GameState& State);
	///	\overload
	PublicState convertToPublic(const CompressedState& State);

	/// \brief	result of a game
	enum GameResult
	{
		NOT_FINISHED,
		PLAYER_1,
		PLAYER_2,
		DRAW
	};

	/// \brief	has the game finished? what's the result?
	GameResult gameFinished(const GameState& State);
	///	\overload
	GameResult gameFinished(const CompressedState& State);

	/*!	\brief	Evaluates moves
		\return	Move score
	*/
	int32_t evaluate(const GameState& Move, const GameState& Previous);
	///	\overload
	int32_t evaluate(const CompressedState& Move, const CompressedState& Previous);
	///	\overload	
	int32_t evaluate(const GameState& Current, CoordU from, CoordU to);
	///	\overload	
	int32_t evaluate(const CompressedState& Current, CoordU from, CoordU to);

	/*!	\brief	compress a GameState
		\param	State	GameState to be compressed
		\return	a CompressedState representing the same game state
	*/
	CompressedState compress(const GameState& State);
	///	\overload	used to avoid pushing the whole CompressedState through the stack
	void compress(const GameState& State, CompressedState& out);
	///	\overload	
	void compress(const GameState& State, CompressedState* out);
	/*!	\brief	decompress a CompressedState
		\param	State	CompressedState to be compressed
		\return	a GameState representing the same game state
	*/
	GameState decompress(const CompressedState& State);
	///	\overload	used to avoid pushing the whole GameState through the stack
	void decompress(const CompressedState& State, GameState& Out);
	///	\overload	
	void decompress(const CompressedState& State, GameState* Out);
	
	/*!	\brief	Generates moves, using GameState
				initialized separately from genMovesC, moves are also generated independently
		\return	Number of moves generated
	*/
	uint16_t genMovesUncomp();
	/*!	\brief	generates moves, using CompressedState
				initialized separately from genMovesC, moves are also generated independently
		\return	number of moves generated
	*/
	uint16_t genMovesComp();

	/*!	\brief	initializes the moves generator
				due to differences in buffers, etc. engine operates separately on GameState and CompressedState
		\param	current		what is the state of the board?
		\param	buffer		buffer for pointers to GameState objects
		\param	bufferSize	size of the buffer
	*/
	void initGen(const GameState& Current, GameState* buffer[], const uint32_t bufferSize);
	///	\overload	due to differences in buffers, etc. engine operates separately on GameState and CompressedState
	void initGen(const CompressedState& Current, CompressedState* buffer[], const uint32_t bufferSize);

	bool isMoveValid(GameState Current, CoordU from, CoordU to);

	GameState makeMove(GameState Current, CoordU from, CoordU to);
}