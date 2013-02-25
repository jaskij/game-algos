#pragma once

#include <stdint.h>

#ifdef _WIN32
	//this symbol must be defined in each engine project
	#ifdef ENGINEDLL
		#define dllFunction __declspec(dllexport) 
	#else
		#define dllFunction __declspec(dllimport)
	#endif
#elif defined __unix__
	#define dllFunction
#endif

namespace engine
{

	/// \brief	state of a single field
	typedef unsigned char FieldState;

	/// \brief object describing a particular game state
	struct GameState;

	/// \brief returns Size of Game State object in bytes
	dllFunction uint32_t getGameStateSize();

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
	dllFunction PublicState convertToPublic(const GameState& State);

	enum GameWinner
	{
		NOT_FINISHED,
		PLAYER_1,
		PLAYER_2,
		DRAW
	};

	dllFunction GameWinner gameFinished(const GameState& State);

	/*!	\brief	Evaluates moves
		\return	Move score
	*/
	dllFunction int32_t evaluate(const GameState& Move, const GameState& Previous) ;
	

	//NOTE: having engine stateless requires passing last 3 parameters. adding state would enable saving them internally
	dllFunction uint32_t genMoves(const GameState& current, GameState* buffer, const uint32_t bufferSize, const GameState& lastGeneratedGameState) ;
	/*!	\brief	Generates moves, using CompressedGamestate
		\return	Number of moves generated
	*/

}