#ifndef ESTAR_WIN32_HPP
#define ESTAR_WIN32_HPP

#include <cmath>

#ifdef WIN32
typedef long ssize_t;

inline double rint(double nr)
{
	double f = floor(nr);
	double c = ceil(nr);
	return (((c-nr) >= (nr-f)) ? f :c);
}

#endif // WIN32

#endif // ESTAR_WIN32_HPP
