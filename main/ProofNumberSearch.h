#pragma once
#include "../interfaces/CommonEngine.h"

namespace algo
{
	class ProofNumberSearch
	{
	public:
	private:
		typedef uint64_t nodeCount;
		struct PNNode
		{
			nodeCount proof;
			nodeCount disproof;
		};
		enum nodeState
		{
			PROVED,
			DISPROOVED,
			UNKNOWN
		};
		bool solve(engine::GameState start, nodeCount maxNodes);
		nodeState evaluatePosition(engine::GameState position);
	};
}
