#pragma once

#include <stdint.h>
#include "../main/utility.h"
#include "../networking/GameState.h"

namespace engine
{

	/// \brief object describing a particular game state
	struct GameState;

	/// \brief a compressed GameState
	struct CompressedState;

	/// \brief converts game state to verbose version, designed to be easily interpreted outside of the engine.
	/// see PublicState definition for details
	net::GameState convertToPublic(const GameState* State);
	/// \overload
	net::GameState convertToPublic(const CompressedState* State);

	/// \brief result of a game
	enum GameResult
	{
		NOT_FINISHED,
		PLAYER_1,
		PLAYER_2,
		DRAW
	};


	/// \brief has the game finished? what's the result?
	GameResult isGameFinished(const GameState* const State);
	/// \overload
	GameResult isGameFinished(const CompressedState* const State);
	/// \overload
	GameResult isGameFinished(const net::GameState& State);

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
	\param current what is the state of the board? expected to remain valid until generating moves is completed
	\param buffer buffer for pointers to GameState objects
	\param bufferSize size of the buffer
	*/
	void initGen(const GameState * const current, GameState* buffer[], const uint32_t bufferSize);
	/// \overload due to differences in buffers, etc. engine operates separately on GameState and CompressedState
	void initGen(const CompressedState * const current, CompressedState* buffer[], const uint32_t bufferSize);


	/// \brief returns default state for current game, usually an empty board
	net::GameState getInitialState();

	/// \brief returns GameState size in bytes
	unsigned getGameStateSize();

	bool isMoveValid(GameState *Current, CoordU from, CoordU to); 	

	GameState* makeMove(GameState* Current, CoordU from, CoordU to);

	/// \brief returns GameState size in bytes 
	unsigned getGameStateSize();

	/// \brief indicates if current player can move a piece located at coords from
	bool canMoveFrom( engine::GameState* currentGameState, CoordU from );

	/// \brief indicates if both origin and destination is required for move, or destination is enough
	bool requiresOriginToMove();

	/// \brief converts net(public) game state to engine's uncompressed variant
	/// \return caller is responsible for freeing returned state
	GameState* netStateToUncomp(net::GameState Public);

	/// \brief returns basic engine configuration as a string
	std::string getConfigString();
}