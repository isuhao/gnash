/* 
 *   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#ifndef _GNASH_FUZZYPIXEL_H
#define _GNASH_FUZZYPIXEL_H

#include "Range2d.h"
#include "gnash.h" // for namespace key
#include "sound_handler_test.h" // for creating the "test" sound handler
#include "types.h" // for rgba class

#include <iostream> 

namespace gnash {

/// An utility class used to compare rgba values with a given tolerance
class FuzzyPixel
{

public:

	friend std::ostream& operator<< (std::ostream& o, const FuzzyPixel& p);

	/// Construct a black, alpha 0 FuzzyPixel with NO tolerance.
	//
	/// No tolerance means that any comparison will fail
	///
	FuzzyPixel()
		:
		_col(0,0,0,0),
		_tol(0)
	{
	}

	/// Construct a FuzzyPixel with given color and tolerance
	//
	/// @param color
	///	The color value
	///
	/// @param tolerance
	///	The tolerance to use in comparisons
	///
	FuzzyPixel(rgba& color, int tolerance=0)
		:
		_col(color),
		_tol(tolerance)
	{
	}

	/// Construct a FuzzyPixel with given values
	//
	/// @param r
	///	The red value.
	///
	/// @param g
	///	The green value.
	///
	/// @param b
	///	The blue value.
	///
	/// @param a
	///	The alpha value.
	///
	FuzzyPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		:
		_col(r, g, b, a),
		_tol(0)
	{
	}

	/// Set the tolerance to use in comparisons
	//
	/// @param tol
	///	The tolerance to use in comparisons
	///
	void setTolerance(int tol)
	{
		_tol = tol;
	}

	/// Compare two FuzzyPixel using the tolerance of the most tolerant of the two
	//
	/// Note that if any of the two operands has 0 tolerance, any equality
	/// comparison will fail.
	///
	bool operator==(const FuzzyPixel& other) const;

	// Return true if a and b are below a given tolerance
	static bool fuzzyEqual(int a, int b, int tol)
	{
		return abs(a-b) <= tol;
	}

private:

	rgba _col;

	// tolerance value
	int _tol;

};

} // namespace gnash

#endif // _GNASH_FUZZYPIXEL_H
