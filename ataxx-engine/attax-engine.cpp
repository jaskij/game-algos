#include "ataxx-engine.h"

#include <list>
#include <cassert>

///	\TODO	utilize JUMP_SHIFT and JUMP_WB_SHIFT more
/// \TODO	make CLONE_SHIFT and CLONE_WB_SHIFT similiar to JUMP_SHIFT

template<class T>
inline T abs(T arg)
{
	return arg < 0 ? -arg : arg;
}

using namespace engine;

const int BOARD_BORDER = 2;
// WB is short for "with border"
const unsigned BOARD_WB_SIZE = 121;
const unsigned BOARD_WB_WIDTH = 11;
const unsigned BOARD_WB_HEIGHT = 11;

const uint8_t COMPRESSED_EMPTY = 0x0;
const uint8_t COMPRESSED_P1 = 0x1;
const uint8_t COMPRESSED_P2 = 0x2;
const uint8_t COMPRESSED_BLOCK = 0x3;

int emptyCount;

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
	for (unsigned i=0; i<BOARD_WB_HEIGHT; i++)
	{
		buffer[i] = GameState::BLOCKED;
		buffer[i + BOARD_WB_WIDTH] = GameState::BLOCKED;
		buffer[i + 9 * BOARD_WB_WIDTH] = GameState::BLOCKED;
		buffer[i + 10 * BOARD_WB_WIDTH] = GameState::BLOCKED;
	}
	for(unsigned i=0; i<BOARD_HEIGHT; i++)
	{
		buffer[i * BOARD_WB_WIDTH] = GameState::BLOCKED;
		buffer[i * BOARD_WB_WIDTH + 1] = GameState::BLOCKED;
		buffer[i * BOARD_WB_WIDTH + BOARD_WB_WIDTH - 2] = GameState::BLOCKED;
		buffer[i * BOARD_WB_WIDTH + BOARD_WB_WIDTH - 1] = GameState::BLOCKED;
		for(unsigned j=0; j<BOARD_WIDTH; j++)
		{
			buffer[(i + BOARD_BORDER) * BOARD_WB_WIDTH + j + BOARD_BORDER] = State.board[i * BOARD_WIDTH + j];
		}
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
	assert(abs(fromX - toX > 2) || abs (fromY - toY) > 2);
	assert(board[toY * BOARD_WB_WIDTH + toX] != GameState::BLOCKED);
	assert(abs(fromX - toX) == 2 || abs(fromY - toY) == 2);
	if(board[fromY * BOARD_WB_WIDTH + fromX] == player)
	{
		return true;
	}
	return false;
}

GameResult engine::gameFinished(const CompressedState& State)
{
	return gameFinished(decompress(State));
}

GameResult engine::gameFinished(const GameState& State)
{
	int empty = countEmpty(State);
	int p1 = 0;
	int p2 = 0;

	for (unsigned i=0; i<BOARD_SIZE; i++)
	{
		switch(State.board[i])
		{
		case GameState::P_1:
			p1++;
			break;
		case GameState::P_2:
			p2++;
			break;
		}
	}

	if (empty == 0){
		if ( p1 > p2 ){
			return PLAYER_1;
		}
		else if ( p2 > p1 ){
			return PLAYER_2;
		}
		else{
			return DRAW;
		}
	}

	GameState::FieldState board[BOARD_WB_SIZE];
	copyWithBorder(State, board);

	for (unsigned y = BOARD_BORDER; y < BOARD_HEIGHT + BOARD_BORDER; y++)
	{
		for (unsigned x = BOARD_BORDER; x < BOARD_WIDTH + BOARD_BORDER; x++)
		{
			if (canJumpTo(State.player, board, x, y))
			{
				return NOT_FINISHED;
			}
			else if (canCloneTo(State.player, board, x, y))
			{
				return NOT_FINISHED;
			}
		}
	}

	if ( p1 > p2 ){
		return PLAYER_1;
	}
	else if ( p2 > p1 ){
		return PLAYER_2;
	}
	else{
		return DRAW;
	}
}

void engine::compress(const GameState& State, CompressedState& Out)
{
	for(unsigned i=0; i<COMPRESSED_SIZE; i++)
	{
		Out.data[i] = 0;
	}
	for(unsigned i=0; i<BOARD_SIZE; i++)
	{
		switch (State.board[i])
		{
		// since Out was emptied at beginning, ignore the EMPTY case
		case GameState::P_1:
			Out.data[i >> 2] |= COMPRESSED_P1 << ((i & 0x3) >> 1);
			break;
		case GameState::P_2:
			Out.data[i >> 2] |= COMPRESSED_P2 << ((i & 0x3) >> 1);
			break;
		case GameState::BLOCKED:
			Out.data[i >> 2] |= COMPRESSED_BLOCK << ((i & 0x3) >> 1);
			break;
		}
	}
	if (State.player == GameState::ONE)
	{
		Out.data[COMPRESSED_SIZE - 1] |= COMPRESSED_P1 << 6;
	}
	else
	{
		Out.data[COMPRESSED_SIZE - 1] |= COMPRESSED_P2 << 6;
	}
}

void engine::compress(const GameState& State, CompressedState* Out)
{
	compress(State, *Out);
}

CompressedState engine::compress(const GameState& State)
{
	CompressedState Ret;
	compress(State, Ret);
	return Ret;
}

void engine::decompress(const CompressedState& State, GameState& Out)
{
	if (((State.data[COMPRESSED_SIZE - 1] >> 6) & 0x3) == COMPRESSED_P1)
	{
		Out.player = GameState::ONE;
	}
	else
	{
		Out.player = GameState::TWO;
	}
	for(unsigned i=0; i<BOARD_SIZE; i++)
	{
		switch ( (State.data[i >> 2] >> ( (i & 0x3) >> 1) ) & 0x3 )
		{
		case COMPRESSED_EMPTY:
			Out.board[i] = GameState::EMPTY;
			break;
		case COMPRESSED_P1:
			Out.board[i] = GameState::P_1;
			break;
		case COMPRESSED_P2:
			Out.board[i] = GameState::P_2;
			break;
		case COMPRESSED_BLOCK:
			Out.board[i] = GameState::BLOCKED;
			break;
		}
	}
}

void engine::decompress(const CompressedState& State, GameState* Out)
{
	decompress(State, *Out);
}

GameState engine::decompress(const CompressedState& State)
{
	GameState Ret;
	decompress(State, Ret);
	return Ret;
}

/// \brief	gives the score for moving adjacent to give field
int32_t fieldByToScore(GameState::Player player, GameState::FieldState (&board)[BOARD_WB_SIZE], unsigned x, unsigned y)
{
	// since in both enumes P_1 == 1 and P_2 == 2, if there is a piece by the other place there
	// this xor will be 3, similarly, if theere is our piece, the xor will be 0
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

int32_t engine::evaluate(const GameState& Current, CoordU from, CoordU to)
{
	int32_t res=0;

	GameState::FieldState board[BOARD_WB_SIZE];
	copyWithBorder(Current, board);

	from.x += BOARD_BORDER;
	from.y += BOARD_BORDER;
	to.x += BOARD_BORDER;
	to.y += BOARD_BORDER;

	bool isJump = false;

	if (abs(to.x - from.x) == 2 || abs(to.y - from.y) == 2)
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

int32_t engine::evaluate(const CompressedState& Current, CoordU from, CoordU to)
{
	return evaluate(decompress(Current), from, to);
}

///	\TODO	rewrite not to use the CoordU version, but use info in Move properly
int32_t engine::evaluate(const GameState& Move, const GameState& Previous)
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

int32_t engine::evaluate(const CompressedState& Move, const CompressedState& Previous)
{
	return evaluate(decompress(Move), decompress(Previous));
}

unsigned genLastX;
unsigned genLastY;
unsigned lastJump;
bool putClone;

GameState::FieldState genBoard[BOARD_WB_SIZE];

CompressedState** genBufferC;
GameState** genBufferU;
uint32_t bufferSize;
GameState::Player genPlayer;
GameState genCurrent;

struct moveTo
{
	bool clone;
	// possible jump from is saved as 1 on bit equal to shift on board with boarders
	uint64_t jumpFrom;
	moveTo(bool clone = false) : clone(clone) { }
};

moveTo possibleMoves[BOARD_WB_SIZE];

const unsigned JUMP_COUNT = 16;
const unsigned JUMP_SHIFT[JUMP_COUNT] = {0, 1, 2, 3, 4,
										BOARD_WIDTH, BOARD_WIDTH + 4,
										2 * BOARD_WIDTH, 2 * BOARD_WIDTH + 4,
										3 * BOARD_WIDTH, 3 * BOARD_WIDTH + 4,
										4 * BOARD_WIDTH, 4 * BOARD_WIDTH + 1, 4 * BOARD_WIDTH + 2, 4 * BOARD_WIDTH + 3, 4 * BOARD_WIDTH + 4};

const unsigned JUMP_WB_SHIFT[JUMP_COUNT] = {0, 1, 2, 3, 4,
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

void engine::initGen(const GameState& Current, GameState* buffer[], const uint32_t buffSize)
{
	genBufferU = buffer;
	initCommon(Current, buffSize);
}

void engine::initGen(const CompressedState& Current, CompressedState* buffer[], const uint32_t buffSize)
{
	genBufferC = buffer;
	initCommon(decompress(Current), buffSize);
}

uint16_t engine::genMovesUncomp()
{
	unsigned i = 0;

	while(genLastX < BOARD_BORDER + BOARD_WIDTH && genLastY < BOARD_BORDER + BOARD_HEIGHT && i < bufferSize)
	{
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
			if(((possibleMoves[genLastY * BOARD_WB_WIDTH + genLastX].jumpFrom >> lastJump) & 1) == 1)
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

uint16_t engine::genMovesComp()
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
			compress(Tmp, genBufferC[i]);
			putClone = true;
		}
		else 
		{
			if(((possibleMoves[genLastY * BOARD_WB_WIDTH + genLastX].jumpFrom >> lastJump) & 1) == 1)
			{
				Tmp . board[(genLastY - 2) * BOARD_WIDTH + genLastX - 2] =
						genCurrent.player == GameState::ONE ?
						GameState::P_1 :
						GameState::P_2;
				Tmp . board[(genLastY - BOARD_BORDER - 2) * BOARD_WIDTH + genLastX - BOARD_BORDER - 2 + JUMP_SHIFT[lastJump]] = GameState::EMPTY;
				i++;
				compress(Tmp, genBufferC[i]);
			}
			lastJump++;
		}
	}
	return i;
}

bool engine::isMoveValid(GameState Current, CoordU from, CoordU to)
{
	GameState::FieldState tmp[BOARD_WB_SIZE];
	copyWithBorder(Current, tmp);

	if(Current.board[to.y * BOARD_WIDTH + to.x] == GameState::BLOCKED)
	{
		return false;
	}

	if(abs(from.x - to.x > 2) || abs(from.y - to.y) > 2)
	{
		return false;
	}

	if(abs(from.x - to.x) == 2  || abs(from.y - to .y) == 2)
	{
		return canJumpFromTo(Current.player, tmp, from.x + BOARD_BORDER, from.y + BOARD_BORDER, to.x + BOARD_BORDER, to.y + BOARD_BORDER);
	}

	else
	{
		return canCloneTo(Current.player, tmp, to.x + BOARD_BORDER, to.y + BOARD_BORDER);
	}
}

void engine::makeMove(GameState Current, GameState& After, CoordU from, CoordU to)
{
	After = Current;
	if(abs(from.x - to.x) == 2 || abs(from.y - to .y) == 2)
	{
		After.board[from.y * BOARD_WIDTH + from.x] = GameState::EMPTY;
	} 
	if(Current.player == GameState::ONE)
	{
		After.board[to.y * BOARD_WIDTH + to.x] = GameState::P_1;
		After.player = GameState::TWO;
	}
	else
	{
		After.board[to.y * BOARD_WIDTH + to.x] = GameState::P_2;
		After.player = GameState::ONE;
	}
}

GameState engine::makeMove(GameState Current, CoordU from, CoordU to)
{
	GameState Ret;
	makeMove(Current,Ret, from, to);
	return Ret;
}
