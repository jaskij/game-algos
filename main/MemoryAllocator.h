/*memory allocator is primarily for allocating game states, which makes it quite simple - allocation is always some 
 *constant size or multiple of it. but algorithm also needs memory for it's structures. question is how much 
 *algorithm-specific this allocation is? is it always a tree? how to manage memory for both purposes?
 *
 */

#pragma once

#include "../interfaces/CommonEngine.h"

class MemoryAllocator
{
public:
	MemoryAllocator();
	~MemoryAllocator();

	//TODO: why allocate doesn't return the pointer? also implement single state allocator/deallocator and use them to replace random malloc/free throughout the code

	/// \brief allocates count states, putting their pointers into an array pointed by where
	void allocateStates(unsigned count, engine::GameState** where);

	/// \brief deallocates count states from array where
	void freeStates(unsigned count, engine::GameState** where);

private:
	unsigned gameStateSize;
};