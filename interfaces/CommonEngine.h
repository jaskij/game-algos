#pragma once

#include <stdint.h>
#include "../main/utility.h"

namespace engine
{

	/// \brief object describing a particular game state
	struct GameState;

	/// \brief a compressed GameState
	struct CompressedState;

	/*! \brief for use with server communications and GUI
	Should be redesigned to make it generic(fitting all games)
	*/
	struct PublicState
	{
		/// \brief whose turn it is
		uint8_t player;
		/// \brief state of the whole board
		uint8_t board[49];
	};

	/// \brief converts game state to verbose version, designed to be easily interpreted outside of the engine.
	/// see PublicState definition for details
	PublicState convertToPublic(const GameState* State);
	/// \overload
	PublicState convertToPublic(const CompressedState* State);

	/// \brief result of a game
	enum GameResult
	{
		NOT_FINISHED,
		PLAYER_1,
		PLAYER_2,
		DRAW
	};


	/// \brief has the game finished? what's the result?
	GameResult gameFinished(const GameState* const State);
	/// \overload
	GameResult gameFinished(const CompressedState* const State);

	/*! \brief Evaluates moves
	\return Move score
	*/
	int32_t evaluate(const GameState& Move, const GameState& Previous);
	/// \overload
	int32_t evaluate(const CompressedState& Move, const CompressedState& Previous);
	/// \overload
	int32_t evaluate(const GameState& Current, CoordU from, CoordU to);
	/// \overload
	int32_t evaluate(const CompressedState& Current, CoordU from, CoordU to);

	


	/*! \brief Generates moves, using GameState
	initialized separately from genMovesComp, moves are also generated independently
	\return Number of moves generated
	*/
	unsigned genMovesUncomp();

	/*! \brief generates moves, using CompressedState
	initialized separately from genMovesUncomp, moves are also generated independently
	\return number of moves generated
	*/
	unsigned genMovesComp();

	/*! \brief initializes the moves generator
	due to differences in buffers, etc. engine operates separately on GameState and CompressedState
	\param current what is the state of the board?
	\param buffer buffer for pointers to GameState objects
	\param bufferSize size of the buffer
	*/
	void initGen(const GameState * const current, GameState* buffer[], const uint32_t bufferSize);
	/// \overload due to differences in buffers, etc. engine operates separately on GameState and CompressedState
	void initGen(const CompressedState * const current, CompressedState* buffer[], const uint32_t bufferSize);


	/*! \brief returns default state for current game, usually an empty board
	  
	  Returned reference is guaranteed to remain valid until next state generation, most likely via Algo.chooseMove
	*/
	GameState* getInitialStateUncomp();
	/// \overload
	CompressedState* getInitialStateComp();

	/// \brief returns GameState size in bytes
	unsigned getGameStateSize();

	bool isMoveValid(GameState *Current, CoordU from, CoordU to); 	

	GameState* makeMove(GameState* Current, CoordU from, CoordU to);

	//TODO: delete after public state is properly implemented
	PublicState convertToPublic(GameState* State);

	/// \brief returns GameState size in bytes 
	unsigned getGameStateSize();

	/// \brief indicates if current player can move a piece located at coords from
	bool canMoveFrom( engine::GameState* currentGameState, CoordU from );
}