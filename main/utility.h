#pragma once

template <class T>
struct Coord
{
	T x, y;
	Coord(T x=0, T y=0) : x(x), y(y)
	{}
};

template <>
struct Coord<unsigned>
{
	unsigned x,y;
	Coord(unsigned x=0, unsigned y=0) : x(x), y(y)
	{}
};

typedef Coord<unsigned> CoordU;


//warning: this function has optimal performance, but is not safe in edge cases. if there is such a possibility, then 
// you should use another function. see http://embeddedgurus.com/stack-overflow/2012/02/the-absolute-truth-about-abs/ 
template<class T>
inline T abs(T arg)
{
	return arg < 0 ? -arg : arg;
}