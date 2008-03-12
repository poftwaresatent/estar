#ifndef ESTAR_WIN32_HPP
#define ESTAR_WIN32_HPP

# ifdef WIN32

// when mixing libsunflower and estar, bail out earlier to avoid multiple definitions
#  ifndef SFL_WIN32_HPP

#   include <cmath>

typedef long ssize_t;

inline double rint(double nr)
{
	double f = floor(nr);
	double c = ceil(nr);
	return (((c-nr) >= (nr-f)) ? f :c);
}

#  endif // SFL_WIN32_HPP
# endif // WIN32
#endif // ESTAR_WIN32_HPP
