#include "AlgoFactory.h"
#include "RandomAlgo.h"
#include <iostream>


AlgoFactory::AlgoFactory(void)
{
}


AlgoFactory::~AlgoFactory(void)
{
}

algo::Algo* AlgoFactory::createAlgo( std::string& name )
{
	if (name == "random")
	{
		return new algo::RandomAlgo();
	} 
	else
	{
		std::cout << "Wrong algorithm name\n";
	}
	return NULL;
}
