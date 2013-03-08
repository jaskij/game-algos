#pragma once
#include "Algo.h"
#include <string>

class AlgoFactory
{
public:
	AlgoFactory(void);
	~AlgoFactory(void);
	algo::Algo* createAlgo(std::string& name);
};

