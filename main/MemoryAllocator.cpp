#include "MemoryAllocator.h"
#include <cstdlib>

MemoryAllocator::~MemoryAllocator()
{
	
}

MemoryAllocator::MemoryAllocator()
{
	gameStateSize = engine::getGameStateSize();
}

void MemoryAllocator::freeStates( unsigned count, engine::GameState** where )
{
	for (unsigned i = 0; i < count; ++i)
	{
		free( where[i] );
	}
}


void MemoryAllocator::allocateStates(unsigned count, engine::GameState** where)
{
	for(unsigned i = 0; i < count; ++i)
	{
		where[i] = (engine::GameState*) malloc( gameStateSize );
	}
}