/*
 * L E S S P T R . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

/*
	Implement a class for comparing pointers so that they can be
	used in maps and sets. This is necessary because pointer
	comparisons have to be done differently on different
	platforms - see Andrew Koenig's "Traps and Pitfalls" article
	in C++ Report, November - December 1996.
*/

#ifndef _BASE_LESSPTR_HPP
#define _BASE_LESSPTR_HPP

#include "base/base.hpp"
//#include <function>



// Used purely as a comparator functor (less_ptr<T>()(a, b)) everywhere in
// this codebase - nothing references the first_argument_type/
// second_argument_type/result_type typedefs std::binary_function supplied,
// so there's no need to inherit from it. That base was removed from the
// standard in C++17 and, unlike libstdc++/libc++, MSVC's STL no longer
// provides any opt-back-in for it at all, making this the portable fix
// rather than chasing compiler-specific compatibility macros.
template< class T >
class less_ptr
{
public:
	bool operator()( T* a, T* b )
	{
		return a < b;
	}
	bool operator()( const T* a, const T* b ) const
	{
		return a < b;
	}
};

#endif
