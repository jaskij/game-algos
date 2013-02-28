#pragma once

#include <stdint.h>

namespace engine
{

	/// \brief object describing a particular game state
	struct GameState;

	///	\brief a compressed GameState
	struct CompressedState;

	/*! \brief	for use with server communications and GUI
		Should be redesigned to make it generic(fitting all games)
	*/
	struct PublicState
	{
		/// \brief	whose turn it is
		uint8_t player;
		/// \brief	state of the whole board
		FieldState board[49];
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

	/*!	\brief	compress a GameState
		\param	state	GameState to be compressed
		\return	a CompressedState representing the same game state
	*/
	CompressedState compress(const GameState& state);
	/*!	\brief	decompress a CompressedState
		\param	state	CompressedState to be compressed
		\return	a GameState representing the same game state
	*/
	GameState decompress(const CompressedState& state);
	

	/*!	\brief	Generates moves, using GameState
				initialized separately from genMovesC, moves are also generated independently
		\return	Number of moves generated
	*/
	uint16_t genMovesD();
	/*!	\brief	generates moves, using CompressedState
				initialized separately from genMovesC, moves are also generated independently
		\return	number of moves generated
	*/
	uint16_t genMovesC();

	/*!	\brief	initializes the moves generator
				due to differences in buffers, etc. engine operates separately on GameState and CompressedState
		\param	current		what is the state of the board?
		\param	buffer		buffer for pointers to GameState objects
		\param	bufferSize	size of the buffer
	*/
	void initGen(const GameState& current, GameState** buffer, const uint32_t bufferSize);
	///	\overload	due to differences in buffers, etc. engine operates separately on GameState and CompressedState
	void initGen(const CompressedState& current, CompressedState** buffer, const uint32_t bufferSize);
}