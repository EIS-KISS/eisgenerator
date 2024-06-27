//SPDX-License-Identifier:         LGPL-3.0-or-later
/* * eisgenerator - a shared library and application to generate EIS spectra
 * Copyright (C) 2022-2024 Carl Philipp Klemm <carl@uvos.xyz>
 *
 * This file is part of eisgenerator.
 *
 * eisgenerator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eisgenerator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with eisgenerator.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <complex>
#include <vector>
#include <valarray>
#include <cassert>
#include <cmath>
#include <filesystem>

/**
 * @brief Datatype for all calculations
 */
typedef float fvalue;

namespace eis
{

/**
* Types used by eisgenerator
* @defgroup TYPES Types
* @{
*/

/**
* @brief Basic singular EIS data point
*/
class DataPoint
{
public:
	std::complex<fvalue> im; /**< Complex impedance */
	fvalue omega; /**< Frequency of the complex impedance */
	DataPoint() = default;
	DataPoint(std::complex<fvalue> imIn, fvalue omegaIn = 0): im(imIn), omega(omegaIn){}
	bool operator<(const DataPoint& in) const
	{
		return omega < in.omega;
	}
	bool operator>(const DataPoint& in) const
	{
		return omega > in.omega;
	}
	bool operator==(const DataPoint& in) const
	{
		return im == in.im;
	}
	DataPoint operator-(const DataPoint& in) const
	{
		DataPoint out(*this);
		out.im = out.im - in.im;
		return out;
	}
	DataPoint operator+(const DataPoint& in) const
	{
		DataPoint out(*this);
		out.im = out.im + in.im;
		return out;
	}
	DataPoint operator/(fvalue in) const
	{
		DataPoint out(*this);
		out.im = out.im / in;
		return out;
	}
	DataPoint operator*(fvalue in) const
	{
		DataPoint out(*this);
		out.im = out.im * in;
		return out;
	}
	DataPoint operator/=(fvalue in)
	{
		im = im / in;
		return *this;
	}
	/**
	* @brief Calculates the absolute value of the complex impedance.
	*
	* @return The absolute value of the complex impedance.
	*/
	fvalue complexVectorLength() const
	{
		return std::sqrt(std::pow(im.real(), 2) + std::pow(im.imag(), 2));
	}
};


/**
* @brief A range
*/
class Range
{
public:
	fvalue start; /**< Start of the range*/
	fvalue end; /**< End of the range*/
	size_t count = 0; /**< Number of elements in the range*/
	size_t step = 0; /**< Currently active step*/
	bool log = false; /**< True if the elements in the range are to be spaced in log10 increments*/

	/**
	* @brief Calculates the distance between elements in the range.
	*
	* Will calculate the log10 of the distance if the range is logarithmic.
	*
	* @return The linear or log distance between steps of this range.
	*/
	fvalue stepSize() const
	{
		fvalue startL = log ? log10(start) : start;
		fvalue endL = log ? log10(end) : end;
		return (endL-startL)/(count-1);
	}

	/**
	* @brief Calculates the value of the current step.
	*
	* @return The value of the current step.
	*/
	fvalue stepValue() const
	{
		return at(step);
	}

	/**
	* @brief Calculates the mean of the start and the end values.
	*
	* @return The mean of the start and the end values.
	*/
	fvalue center() const
	{
		return (start+end)/2;
	}

	/**
	* @brief Calculates the value at the given index.
	*
	* @return The value at the given index.
	*/
	fvalue at(size_t index) const
	{
		assert(index < count || (index == 0 && count == 0));
		if(count < 2)
			return start;
		return log ? pow(10, stepSize()*index+log10(start)) : stepSize()*index+start;
	}

	fvalue operator[](size_t index) const
	{
		return at(index);
	}
	Range operator*(fvalue in) const
	{
		return Range(start*in, end*in, count, log);
	}
	Range operator/(fvalue in) const
	{
		return operator*(static_cast<fvalue>(1.0)/in);
	}
	Range operator*(int in) const
	{
		return operator*(static_cast<fvalue>(in));
	}
	Range operator/(int in) const
	{
		return operator*(static_cast<fvalue>(1.0)/in);
	}
	Range(fvalue startI, fvalue endI, size_t countI, bool logI = false): start(startI), end(endI), count(countI), log(logI){}
	Range() = default;

	/**
	* @brief Prints the range to stdout via eis::Log.
	*
	* @param level The eis::Log::Level to print at.
	*/
	void print(int level) const;

	/**
	* @brief Gets a machine parseable string encoding this range.
	*
	* @return The string.
	*/
	std::string getString() const;

	/**
	* @brief Checks if the values of this range are sane/
	*
	* This checks for some common errors like having a end < being.
	*
	* @return true if sane, false if not.
	*/
	bool isSane() const;

	/**
	* @brief This function constructs a vector that contains all elements of this range.
	*
	* @return The vector with the elements of this range.
	*/
	std::vector<fvalue> getRangeVector() const;

	/**
	* @brief This function creates a range from the parseable string.
	*
	* @throw std::invalid_argument when the string syntax is incorrect.
	* @return The range that was parsed from the string.
	*/
	[[nodiscard]] static Range fromString(std::string str, size_t count);

	/**
	* @brief This function creates a vector ranges from the parseable parameter array string.
	*
	* @throw std::invalid_argument when the string syntax is incorrect
	* @return The vector of ranges that was parsed from the string.
	*/
	[[nodiscard]] static std::vector<Range> rangesFromParamString(const std::string& paramStr, size_t count);
};

class parse_errror: public std::exception
{
	std::string whatStr;
public:
	parse_errror(const std::string& whatIn): whatStr(whatIn)
	{}
	virtual const char* what() const noexcept override
	{
		return whatStr.c_str();
	}
};

class file_error: public std::exception
{
	std::string whatStr;
public:
	file_error(const std::string& whatIn): whatStr(whatIn)
	{}
	virtual const char* what() const noexcept override
	{
		return whatStr.c_str();
	}
};

/**
* @brief Returns the a vector of DataPoints as a pair of valarrays.
*
* @param data the data to transform to valarrays
* @return A pair of valarrays, first the real part and second the imaginary part.
*/
std::pair<std::valarray<fvalue>, std::valarray<fvalue>> eisToValarrays(const std::vector<eis::DataPoint>& data);

/** @} */


struct VersionFixed
{
	int major;
	int minor;
	int patch;
};

const VersionFixed& getVersion();

}

std::ostream &operator<<(std::ostream &s, eis::DataPoint const& dp);

std::ostream &operator<<(std::ostream &s, eis::Range const& range);

#include "spectra.h"
