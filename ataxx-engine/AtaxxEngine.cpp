#include "AtaxxEngine.h"

#include <list>
#include <cassert>
#include <cstdlib>

/// \TODO utilize JUMP_SHIFT and JUMP_WB_SHIFT more
/// \TODO make CLONE_SHIFT and CLONE_WB_SHIFT similar to JUMP_SHIFT



using std::max;
using std::min;

namespace engine
{
	static const int BOARD_BORDER = 2;
	// WB is short for "with border"
	static const unsigned BOARD_WB_SIZE = 121;
	static const unsigned BOARD_WB_WIDTH = 11;
	static const unsigned BOARD_WB_HEIGHT = 11;

	static const int BOARD_SINGLE_BORDER = 1;
	//WSB is short for "with single border"
	static const unsigned BOARD_WSB_SIZE = 81;
	static const unsigned BOARD_WSB_WIDTH = 9;
	static const unsigned BOARD_WSB_HEIGHT = 9;

	static const uint8_t COMPRESSED_EMPTY = 0x0;
	static const uint8_t COMPRESSED_P1 = 0x1;
	static const uint8_t COMPRESSED_P2 = 0x2;
	static const uint8_t COMPRESSED_BLOCK = 0x3;

	static int emptyCount;

	static const int JUMP_LENGTH = 2;

	inline bool diffEqual(unsigned coord1, unsigned coord2, int howMuch = JUMP_LENGTH)
	{
		return coord1 + howMuch == coord2 || coord2 + howMuch == coord1;
	}

	inline bool diffBiggerThan(unsigned coord1, unsigned coord2, int howMuch = JUMP_LENGTH)
	{
		return coord1 + howMuch < coord2 || coord2 + howMuch < coord1;
	}

	int countEmpty(const GameState& State)
	{
		int res = 0;
		for ( unsigned i=0; i<BOARD_SIZE; i++ )
		{
			if(State.board[i] == GameState::EMPTY)
			{
				res++;
			}
		}
		return res;
	}

	void copyWithBorder(const GameState& State, GameState::FieldState (&buffer)[BOARD_WB_SIZE])
	{
		memset(buffer, GameState::BLOCKED, BOARD_WB_SIZE * sizeof(GameState::FieldState));
		for(unsigned i = 0; i < BOARD_HEIGHT; ++i)
		{
			memcpy(buffer + BOARD_BORDER + (i + BOARD_BORDER) * BOARD_WB_WIDTH, State.board + i * BOARD_WIDTH, 
				BOARD_WIDTH * sizeof(GameState::FieldState));
		}
	}
	//TODO: get rid of code duplication. probably template?
	void copyWithSingleBorder(const GameState& State, GameState::FieldState (&buffer)[BOARD_WSB_SIZE])
	{
		memset(buffer, GameState::BLOCKED, BOARD_WSB_SIZE * sizeof(GameState::FieldState));
		for(unsigned i = 0; i < BOARD_HEIGHT; ++i)
		{
			memcpy(buffer + BOARD_SINGLE_BORDER + (i + BOARD_SINGLE_BORDER) * BOARD_WSB_WIDTH, State.board + i * BOARD_WIDTH, 
				BOARD_WIDTH * sizeof(GameState::FieldState));
		}
	}

	void stripSingleBorderAndCopy( GameState* State, GameState::FieldState* board )
	{
		for(unsigned i = 0; i < BOARD_HEIGHT; ++i)
		{
			memcpy(State->board + i * BOARD_WIDTH, board + BOARD_SINGLE_BORDER + (i + BOARD_SINGLE_BORDER) * BOARD_WSB_WIDTH,
				BOARD_WIDTH * sizeof(GameState::FieldState));
		}
	}

	// both canJumpTo and canClonteTo accept coordinates on the bordered board
	bool canJumpTo(GameState::Player player, GameState::FieldState (&board)[BOARD_WB_SIZE], unsigned x, unsigned y)
	{
		for(unsigned i=0; i<5; i++)
		{
			if (board[(y - 2) * BOARD_WB_WIDTH + x - 2 + i] == player)
			{
				return true;
			}
			if (board[(y + 2) * BOARD_WB_WIDTH + x - 2 + i] == player)
			{
				return true;
			}
		}
		for (unsigned i=0; i<3; i++)
		{
			if (board[(y - 1 + i) * BOARD_WB_WIDTH + x - 2] == player)
			{
				return true;
			}
			if (board[(y - 1 + i) * BOARD_WB_WIDTH + x + 2] == player)
			{
				return true;
			}
		}

		return false;
	}

	bool canCloneTo(GameState::Player player, GameState::FieldState (&board)[BOARD_WB_SIZE], unsigned x, unsigned y)
	{
		for(unsigned i=0; i<3; i++)
		{
			if (board[(y - 1) * BOARD_WB_WIDTH + x - 1 + i] == player)
			{
				return true;
			}
			if (board[(y + 1) * BOARD_WB_WIDTH + x - 1 + i] == player)
			{
				return true;
			}
		}
		if (board[(y) * BOARD_WB_WIDTH + x - 1] == player)
		{
			return true;
		}
		if (board[(y) * BOARD_WB_WIDTH + x + 1] == player)
		{
			return true;
		}

		return false;
	}

	// this version has asserts for checking if coordinates are proper, as it is meant as internal helper
	// also takes coordinates for bordered board
	bool canJumpFromTo(GameState::Player player, GameState::FieldState (&board)[BOARD_WB_SIZE], unsigned fromX, unsigned fromY, unsigned toX, unsigned toY)
	{
		assert(!diffBiggerThan(fromX, toX) && !diffBiggerThan(fromY, toY));
		assert(board[toY * BOARD_WB_WIDTH + toX] != GameState::BLOCKED);
		assert(diffEqual(fromX, toX) || diffEqual(fromY, toY));
		if(board[fromY * BOARD_WB_WIDTH + fromX] == player && board[toY * BOARD_WB_WIDTH + toX] == GameState::EMPTY)
		{
			return true;
		}
		return false;
	}

	GameResult isGameFinished(const CompressedState* const State)
	{
		GameState decompressed;
		decompress(State, &decompressed);
		return isGameFinished(&decompressed);
	}

	GameResult isGameFinished(const GameState* const State)
	{
		int empty = countEmpty(*State);
		int p1 = 0;
		int p2 = 0;

		for (unsigned i=0; i<BOARD_SIZE; i++)
		{
			switch(State->board[i])
			{
			case GameState::P_1:
				p1++;
				break;
			case GameState::P_2:
				p2++;
				break;
			}
		}

		if (empty == 0)
		{
			if ( p1 > p2 )
			{
				return PLAYER_1;
			}
			else if ( p2 > p1 )
			{
				return PLAYER_2;
			}
			else
			{
				return DRAW;
			}
		}

		GameState::FieldState board[BOARD_WB_SIZE];
		copyWithBorder(*State, board);

		for (unsigned y = BOARD_BORDER; y < BOARD_HEIGHT + BOARD_BORDER; y++)
		{
			for (unsigned x = BOARD_BORDER; x < BOARD_WIDTH + BOARD_BORDER; x++)
			{
				if (canJumpTo(State->player, board, x, y))
				{
					return NOT_FINISHED;
				}
				else if (canCloneTo(State->player, board, x, y))
				{
					return NOT_FINISHED;
				}
			}
		}

		if ( p1 > p2 )
		{
			return PLAYER_1;
		}
		else if ( p2 > p1 )
		{
			return PLAYER_2;
		}
		else
		{
			return DRAW;
		}
	}

	CompressedState * compress(const GameState* const State, CompressedState * Out)
	{
		for(unsigned i=0; i<COMPRESSED_SIZE; i++)
		{
			Out->data[i] = 0;
		}
		for(unsigned i=0; i<BOARD_SIZE; i++)
		{
			switch (State->board[i])
			{
				// since Out was emptied at beginning, ignore the EMPTY case
			case GameState::P_1:
				Out->data[i >> 2] |= COMPRESSED_P1 << ((i & 0x3) >> 1);
				break;
			case GameState::P_2:
				Out->data[i >> 2] |= COMPRESSED_P2 << ((i & 0x3) >> 1);
				break;
			case GameState::BLOCKED:
				Out->data[i >> 2] |= COMPRESSED_BLOCK << ((i & 0x3) >> 1);
				break;
			}
		}
		if (State->player == GameState::ONE)
		{
			Out->data[COMPRESSED_SIZE - 1] |= COMPRESSED_P1 << 6;
		}
		else
		{
			Out->data[COMPRESSED_SIZE - 1] |= COMPRESSED_P2 << 6;
		}
		return Out;
	}

	GameState* decompress(const CompressedState * const State, GameState * Out)
	{
		if (((State->data[COMPRESSED_SIZE - 1] >> 6) & 0x3) == COMPRESSED_P1)
		{
			Out->player = GameState::ONE;
		}
		else
		{
			Out->player = GameState::TWO;
		}
		for(unsigned i=0; i<BOARD_SIZE; i++)
		{
			switch ( (State->data[i >> 2] >> ( (i & 0x3) >> 1) ) & 0x3 )
			{
			case COMPRESSED_EMPTY:
				Out->board[i] = GameState::EMPTY;
				break;
			case COMPRESSED_P1:
				Out->board[i] = GameState::P_1;
				break;
			case COMPRESSED_P2:
				Out->board[i] = GameState::P_2;
				break;
			case COMPRESSED_BLOCK:
				Out->board[i] = GameState::BLOCKED;
				break;
			}
		}
		return Out;
	}


	/// \brief gives the score for moving adjacent to give field
	int32_t fieldByToScore(GameState::Player player, GameState::FieldState (&board)[BOARD_WB_SIZE], unsigned x, unsigned y)
	{
		// since in both enumes P_1 == 1 and P_2 == 2, if there is a piece by the other place there
		// this xor will be 3, similarly, if there is our piece, the xor will be 0
		switch (player ^ board[y * BOARD_WB_WIDTH + x])
		{
		case 0x3:
			return 10;
		case 0x0:
			return 4;
		default:
			return 0;
		}
	}

	int32_t evaluate(const GameState& Current, CoordU from, CoordU to)
	{
		int32_t res=0;

		GameState::FieldState board[BOARD_WB_SIZE];
		copyWithBorder(Current, board);

		from.x += BOARD_BORDER;
		from.y += BOARD_BORDER;
		to.x += BOARD_BORDER;
		to.y += BOARD_BORDER;

		bool isJump = false;

		if (diffEqual(from.x, to.x) || diffEqual(from.y, to.y))
		{
			isJump = true;
			res = 7;
		}

		for(int i=-1; i <= 1; i++)
		{
			res += fieldByToScore(Current.player, board, to.x + i, to.y - 1);
			res += fieldByToScore(Current.player, board, to.x + i, to.y + 1);
			if(isJump)
			{
				if (Current.player == board[(from.y - 1) * BOARD_WB_WIDTH + from.x + i])
				{
					res -= 4;
				}
				if (Current.player == board[(from.y + 1) * BOARD_WB_WIDTH + from.x + i])
				{
					res -= 4;
				}
			}
		}
		res += fieldByToScore(Current.player, board, to.x - 1, to.y);
		res += fieldByToScore(Current.player, board, to.x + 1, to.y);
		if(isJump)
		{
			if (Current.player == board[(from.y) * BOARD_WB_WIDTH + from.x - 1])
			{
				res -= 4;
			}
			if (Current.player == board[(from.y) * BOARD_WB_WIDTH + from.x + 1])
			{
				res -= 4;
			}
		}

		if(res < 0)
		{
			return 0;
		}
		else
		{
			return res;
		}
	}

	int32_t evaluate(const CompressedState& Current, CoordU from, CoordU to)
	{
		GameState CurrentUncomp;
		return evaluate(*decompress(&Current, &CurrentUncomp), from, to);
	}

	/// \TODO rewrite not to use the CoordU version, but use info in Move properly
	int32_t evaluate(const GameState& Move, const GameState& Previous)
	{
		CoordU to, from;
		bool setFrom = false;
		for(unsigned i = 0; i<BOARD_SIZE; i++)
		{
			if (Previous.board[i] == GameState::EMPTY && Move.board[i] != GameState::EMPTY)
			{
				to.y = i / BOARD_WIDTH;
				to.x = i % BOARD_WIDTH;
			}
			else if (Move.board[i] == GameState::EMPTY && Previous.board[i] != GameState::EMPTY)
			{
				from.y = i / BOARD_WIDTH;
				from.x = i % BOARD_WIDTH;
				setFrom = true;
			}
		}

		if(!setFrom)
		{
			GameState::FieldState board[BOARD_WB_SIZE];
			copyWithBorder(Previous, board);

			// since the border is 2, that's the short (and crappy) loop ver
			// should be written more nicely, but this whole func is to be rewritten
			for(int i=1; i <= 3; i++)
			{
				if (board[(to.y + 1) * BOARD_WIDTH + to.x + i] == Previous.player)
				{
					from.y = to.y + 1;
					from.x = to.x + i;
					break;
				}
				else if (board[(to.y + 3) * BOARD_WIDTH + to.x + i] == Previous.player)
				{
					from.y = to.y + 3;
					from.x = to.x + i;
					break;
				}
			}
			if (board[(to.y + 2) * BOARD_WIDTH + to.x + 1] == Previous.player)
			{
				from.y = to.y + 2;
				from.x = to.x + 1;
			}
			if (board[(to.y + 2) * BOARD_WIDTH + to.x + 3] == Previous.player)
			{
				from.y = to.y + 2;
				from.x = to.x + 3;
			}
		}

		return evaluate(Move, from, to);
	}

	int32_t evaluate(const CompressedState& Move, const CompressedState& Previous)
	{
		GameState MoveUncomp, PrevUncomp;
		return evaluate(*decompress(&Move, &MoveUncomp), *decompress(&Previous, &PrevUncomp));
	}

	static unsigned genLastX;
	static unsigned genLastY;
	static unsigned lastJump;
	static bool putClone;

	static GameState::FieldState genBoard[BOARD_WB_SIZE];

	static CompressedState** genBufferC;
	static GameState** genBufferU;
	static uint32_t bufferSize;
	static GameState::Player genPlayer;
	static GameState genCurrent;

	struct moveTo
	{
		bool clone;
		// possible jump from is saved as 1 on bit equal to shift on board with boarders
		uint64_t jumpFrom;
		moveTo(bool clone = false) : clone(clone) { }
	};

	moveTo possibleMoves[BOARD_WB_SIZE];

	static const unsigned JUMP_COUNT = 16;
	static const unsigned JUMP_SHIFT[JUMP_COUNT] = {0, 1, 2, 3, 4,
		BOARD_WIDTH, BOARD_WIDTH + 4,
		2 * BOARD_WIDTH, 2 * BOARD_WIDTH + 4,
		3 * BOARD_WIDTH, 3 * BOARD_WIDTH + 4,
		4 * BOARD_WIDTH, 4 * BOARD_WIDTH + 1, 4 * BOARD_WIDTH + 2, 4 * BOARD_WIDTH + 3, 4 * BOARD_WIDTH + 4};

	static const unsigned JUMP_WB_SHIFT[JUMP_COUNT] = {0, 1, 2, 3, 4,
		BOARD_WB_WIDTH, BOARD_WB_WIDTH + 4,
		2 * BOARD_WB_WIDTH, 2 * BOARD_WB_WIDTH + 4,
		3 * BOARD_WB_WIDTH, 3 * BOARD_WB_WIDTH + 4,
		4 * BOARD_WB_WIDTH, 4 * BOARD_WB_WIDTH + 1, 4 * BOARD_WB_WIDTH + 2, 4 * BOARD_WB_WIDTH + 3, 4 * BOARD_WB_WIDTH + 4};

	inline void markJumpFrom(uint64_t& jumpMask, unsigned toX, unsigned toY, unsigned fromX, unsigned fromY)
	{
		unsigned shift = (toY - fromY - 2) * BOARD_WB_WIDTH + toX - fromX - 2;
		jumpMask |= (uint64_t)1 << shift;
	}

	inline void markJumpFrom(uint64_t& jumpMask, unsigned toX, unsigned toY, unsigned shiftStart)
	{
		unsigned shift = (toY * BOARD_WB_WIDTH) + toX - shiftStart;
		jumpMask |= (uint64_t)1 << shift;
	}

	void initCommon(const GameState& Current, const uint32_t buffSize)
	{
		copyWithBorder(Current, genBoard);
		bufferSize = buffSize;	
		genLastX = BOARD_BORDER;
		genLastY = BOARD_BORDER;
		genPlayer = Current.player;
		putClone = false;
		lastJump = 0;

		for (unsigned x = BOARD_BORDER; x < BOARD_WIDTH + BOARD_BORDER; x++)
		{
			for(unsigned y = BOARD_BORDER; y < BOARD_HEIGHT + BOARD_BORDER; y++)
			{
				possibleMoves[y * BOARD_WB_WIDTH + x].clone = false;
				possibleMoves[y * BOARD_WB_WIDTH + x].jumpFrom = 0;
				if(Current.board[y * BOARD_WB_WIDTH + x] != GameState::EMPTY)
				{
					continue;
				}

				unsigned tmpX;
				unsigned tmpY;
				const unsigned shiftStart = (y - 2) * BOARD_WB_WIDTH + x - 2;

				possibleMoves[y * BOARD_WB_WIDTH + x].clone = canCloneTo(Current.player, genBoard, x, y);
				tmpY = y - 2;
				for(tmpX = x - 2; tmpX <= x + 2; tmpX ++)
				{
					if(genBoard[tmpX * BOARD_WB_WIDTH + tmpY] == Current.player)
					{
						markJumpFrom(possibleMoves[y * BOARD_WB_WIDTH + x].jumpFrom, x, y, shiftStart);
					}
				}
				tmpY = y + 2;
				for(tmpX = x - 2; tmpX <= x + 2; tmpX ++)
				{
					if(genBoard[tmpX * BOARD_WB_WIDTH + tmpY] == Current.player)
					{
						markJumpFrom(possibleMoves[y * BOARD_WB_WIDTH + x].jumpFrom, x, y, shiftStart);
					}
				}
				tmpX = x - 2;
				for(tmpY = y - 1; tmpY <= y + 1; tmpY ++)
				{
					if(genBoard[tmpX * BOARD_WB_WIDTH + tmpY] == Current.player)
					{
						markJumpFrom(possibleMoves[y * BOARD_WB_WIDTH + x].jumpFrom, x, y, shiftStart);
					}
				}
				tmpX = x + 2;
				for(tmpY = y - 1; tmpY <= y + 1; tmpY ++)
				{
					if(genBoard[tmpX * BOARD_WB_WIDTH + tmpY] == Current.player)
					{
						markJumpFrom(possibleMoves[y * BOARD_WB_WIDTH + x].jumpFrom, x, y, shiftStart);
					}
				}
			}
		}
	}

	void initGen(const GameState * const Current, GameState* buffer[], const uint32_t buffSize)
	{
		genBufferU = buffer;
		initCommon(*Current, buffSize);
	}

	void initGen(const CompressedState * const Current, CompressedState* buffer[], const uint32_t buffSize)
	{
		genBufferC = buffer;
		GameState CurrentUncomp;
		initCommon(*decompress(Current, &CurrentUncomp), buffSize);
	}

	unsigned genMovesUncomp()
	{
		unsigned i = 0;

		while(genLastX < BOARD_BORDER + BOARD_WIDTH && genLastY < BOARD_BORDER + BOARD_HEIGHT && i < bufferSize)
		{
			//that is uninitialized read only thingy - wtf?
			*genBufferU[i] = genCurrent;
			if(genCurrent.player == GameState::ONE)
			{
				genBufferU[i] -> player = GameState::TWO;
			}
			else
			{
				genBufferU[i] -> player = GameState::ONE;
			}
			if(lastJump == JUMP_COUNT)
			{
				genLastX++;
				if (genLastX == BOARD_BORDER + BOARD_WIDTH)
				{
					genLastX = BOARD_BORDER;
					genLastY ++;
				}
				putClone = false;
				lastJump = 0;
			}
			if (!putClone)
			{
				if(possibleMoves[genLastY * BOARD_WB_WIDTH + genLastX].clone)
				{
					// what about explicitly assigning P_1 to ONE and P_2 to TWO to eliminate if?
					genBufferU[i] -> board[(genLastY - 2) * BOARD_WIDTH + genLastX - 2] =
						genCurrent.player == GameState::ONE ?
						GameState::P_1 :
						GameState::P_2;
				}
				i++;
				putClone = true;
			}
			else
			{
				if(((possibleMoves[genLastY * BOARD_WB_WIDTH + genLastX].jumpFrom >> JUMP_WB_SHIFT[lastJump]) & 1) == 1)
				{
					genBufferU[i] -> board[(genLastY - 2) * BOARD_WIDTH + genLastX - 2] =
						genCurrent.player == GameState::ONE ?
						GameState::P_1 :
					GameState::P_2;
					genBufferU[i] -> board[(genLastY - BOARD_BORDER - 2) * BOARD_WIDTH + genLastX - BOARD_BORDER - 2 + JUMP_SHIFT[lastJump]] = GameState::EMPTY;
					i++;
				}
				lastJump++;
			}
		}
		return i;
	}

	unsigned genMovesComp()
	{
		unsigned i = 0;

		GameState Tmp;

		while(genLastX < BOARD_BORDER + BOARD_WIDTH && genLastY < BOARD_BORDER + BOARD_HEIGHT && i < bufferSize)
		{
			Tmp = genCurrent;
			if(genCurrent.player == GameState::ONE)
			{
				Tmp . player = GameState::TWO;
			}
			else
			{
				Tmp . player = GameState::ONE;
			}
			if(lastJump == JUMP_COUNT)
			{
				genLastX++;
				if (genLastX == BOARD_BORDER + BOARD_WIDTH)
				{
					genLastX = BOARD_BORDER;
					genLastY ++;
				}
				putClone = false;
				lastJump = 0;
			}
			if (!putClone)
			{
				if(possibleMoves[genLastY * BOARD_WB_WIDTH + genLastX].clone)
				{
					Tmp . board[(genLastY - 2) * BOARD_WIDTH + genLastX - 2] =
						genCurrent.player == GameState::ONE ?
						GameState::P_1 :
					GameState::P_2;
				}
				i++;
				compress(&Tmp, genBufferC[i]);
				putClone = true;
			}
			else
			{
				if(((possibleMoves[genLastY * BOARD_WB_WIDTH + genLastX].jumpFrom >> JUMP_WB_SHIFT[lastJump]) & 1) == 1)
				{
					Tmp . board[(genLastY - 2) * BOARD_WIDTH + genLastX - 2] =
						genCurrent.player == GameState::ONE ?
						GameState::P_1 :
					GameState::P_2;
					Tmp . board[(genLastY - BOARD_BORDER - 2) * BOARD_WIDTH + genLastX - BOARD_BORDER - 2 + JUMP_SHIFT[lastJump]] = GameState::EMPTY;
					i++;
					compress(&Tmp, genBufferC[i]);
				}
				lastJump++;
			}
		}
		return i;
	}

	bool isMoveValid(GameState *Current, CoordU from, CoordU to)
	{
		assert(isCoordValid(from));//it should have been already checked at this point
		if(!isCoordValid(to))
			return false;
		GameState::FieldState tmp[BOARD_WB_SIZE];
		copyWithBorder(*Current, tmp);

		if(Current->board[to.y * BOARD_WIDTH + to.x] == GameState::BLOCKED)
		{
			return false;
		}

		if(diffBiggerThan(from.x, to.x) || diffBiggerThan(from.y, to.y))
		{
			return false;
		}

		if(diffEqual(from.x, to.x) || diffEqual(from.y, to.y))
		{
			return canJumpFromTo(Current->player, tmp, from.x + BOARD_BORDER, from.y + BOARD_BORDER, to.x + BOARD_BORDER, to.y + BOARD_BORDER);
		}
		else
		{
			return canCloneTo(Current->player, tmp, to.x + BOARD_BORDER, to.y + BOARD_BORDER);
		}
	}

	GameState* makeMove(GameState *Current, CoordU from, CoordU to)
	{
		assert(isMoveValid(Current, from, to));
		if(diffEqual(from.x, to.x) || diffEqual(from.y, to.y))
		{
			Current->board[from.y * BOARD_WIDTH + from.x] = GameState::EMPTY;
		}
		if(Current->player == GameState::ONE)
		{
			Current->board[to.y * BOARD_WIDTH + to.x] = GameState::P_1;
			Current->player = GameState::TWO;
		}
		else
		{
			Current->board[to.y * BOARD_WIDTH + to.x] = GameState::P_2;
			Current->player = GameState::ONE;
		}
		switchPiecesOwner(Current, to);
		return Current;
	}

	void switchPiecesOwner(GameState* State, CoordU& newPiece)
	{
		GameState::FieldState board[BOARD_WSB_SIZE];
		copyWithSingleBorder(*State, board);
		GameState::FieldState oldOwner =  
			board[newPiece.x + BOARD_SINGLE_BORDER + BOARD_WSB_WIDTH * (newPiece.y + BOARD_SINGLE_BORDER)]  == GameState::P_1 ?
				GameState::P_2 : GameState::P_1;
		GameState::FieldState newOwner = board[newPiece.x + BOARD_SINGLE_BORDER + BOARD_WSB_WIDTH * (newPiece.y + BOARD_SINGLE_BORDER)];
		for(int i = -1; i <= 1; ++i)
		{
			for (int ii = -1; ii <= 1; ++ii)
			{
				unsigned currentField = newPiece.x + i + BOARD_SINGLE_BORDER + BOARD_WSB_WIDTH * (newPiece.y + BOARD_SINGLE_BORDER + ii);
				if(board[currentField] == oldOwner)
				{
					board[currentField] = newOwner;
				}
			}
		}
		stripSingleBorderAndCopy(State, board);
	}

	CompressedState* getInitialStateComp()
	{
		//see above
		CompressedState * retVal = (CompressedState*)malloc(sizeof(CompressedState));
		compress(getInitialStateUncomp(), retVal);
		return retVal;
	}

	GameState* getInitialStateUncomp()
	{
		GameState * retVal = (GameState*)malloc(sizeof(GameState)) ;
		for(unsigned i = 0; i < BOARD_SIZE;++i)
		{
			retVal->board[i] = GameState::EMPTY;
		}
		retVal->board[0] = GameState::P_1;
		retVal->board[BOARD_SIZE - 1] = GameState::P_1;
		retVal->board[BOARD_WIDTH - 1] = GameState::P_2;
		retVal->board[BOARD_SIZE - BOARD_WIDTH] = GameState::P_2;
		retVal->player = GameState::ONE;
		return retVal;
	}

	PublicState convertToPublic( GameState* State )
	{
		PublicState PubState;
		for(unsigned i = 0; i < BOARD_SIZE;++i)
		{
			PubState.board[i] = State->board[i];
		}
		PubState.player = State->player;
		return PubState;
	}

	unsigned getGameStateSize()
	{
		return sizeof(GameState);
	}

	bool canMoveFrom( engine::GameState* currentGameState, CoordU from )
	{
		return isCoordValid(from) && currentGameState->board[from.x + BOARD_WIDTH * from.y] == currentGameState->player;
	}

	bool requiresOriginToMove()
	{
		return true;
	}

	/// checks if Coord lies within the board
	bool isCoordValid(CoordU coord)
	{
		return coord.x < BOARD_WIDTH && coord.y < BOARD_HEIGHT;
	}
}