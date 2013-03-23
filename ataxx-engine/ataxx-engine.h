#pragma once

namespace ataxx
{

	/// \brief	state of a single field
	typedef unsigned char FieldState;

	/// \brief	State of the whole game
	struct GameState
	{
		/// \brief	whose turn it is
		unsigned char player;
		/// \brief	state of the whole board
		FieldState board[49];
	};

	/// \brief	Compressed game state, for memory-intensive algos
	struct CompressedGameState{
		unsigned char fields[13];
		unsigned char operator[](unsigned char at);
	};

	/// \brief	for use with server communications and GUI
	typedef GameState PublicState;

	class ataxxEngine
	{
	private:
	public:
		PublicState convertToPublic(const GameState& State);
		bool gameFinished(const GameState& State) const;
		/*!	\brief	Evaluates moves
			\return	Move score
		*/
		int evaluate(const GameState& Move, const GameState& Previous) const;
		/*!	\brief	Generates moves
			\return	Number of moves generated
		*/

		unsigned int genMoves(GameState& current, GameState* container) const;
		/*!	\brief	Generates moves, using CompressedGamestate
			\return	Number of moves generated
		*/
		unsigned int genCompressedMoves(CompressedGameState& current, CompressedGameState* container) const;
		unsigned int moveCount(const GameState& state) const;
		unsigned int approxMaxMoves();
		ataxxEngine();
		~ataxxEngine();
	};

}