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
#include <vector>
#include "eistype.h"

namespace eis
{
	/**
	* Various math functions performed on eisgenerator types
	* @defgroup MATH Math
	* @{
	*/

	/**
	* @brief Calculates the element wise absolute gradient at the given point of the data given.
	*
	* @param data The data to calculate the gradient on.
	* @param index The position in the data to calculate the gradient at.
	* @return The element wise absolute gradient abs(re(grad(a)))+abs(im(grad(a)))i.
	*/
	std::complex<fvalue> absGrad(const std::vector<eis::DataPoint>& data, size_t index);

	/**
	* @brief Calculates derivative at the given point of the data given.
	*
	* @param data The data to calculate the gradient on.
	* @param index The position in the data to calculate the gradient at.
	* @return The derivative.
	*/
	fvalue grad(const std::vector<fvalue>& data, const std::vector<fvalue>& omega, size_t index);

	/**
	* @brief Calculates the gradient at the given point of the data given.
	*
	* @param data The data to calculate the gradient on.
	* @param index The position in the data to calculate the gradient at.
	* @return The gradient.
	*/
	std::complex<fvalue> grad(const std::vector<eis::DataPoint>& data, size_t index);

	/**
	* @brief Calculates the mean of the given data.
	*
	* @param data The data to calculate the mean of.
	* @return The mean
	*/
	fvalue mean(const std::vector<fvalue>& data);

	/**
	* @brief Calculates the mean of the given data.
	*
	* @param data The data to calculate the mean of.
	* @return The mean
	*/
	std::complex<fvalue> mean(const std::vector<eis::DataPoint>& data);

	/**
	* @brief Calculates the median of the given data.
	*
	* @param data The data to calculate the median of.
	* @return The median.
	*/
	fvalue median(std::vector<fvalue> data);

	/**
	* @brief Calculates the median of the given data.
	*
	* @param data The data to calculate the median of.
	* @return The median.
	*/
	std::complex<fvalue> median(const std::vector<eis::DataPoint>& data);


	/**
	* @brief Calculates the difference between two spectra.
	*
	* this function calculates an element wise a = a-b and ingores omega
	*
	* @param a The first operand, where b is subtracted
	* @param b The second operand that is subtracted from the first, must be the same size as a
	*/
	void difference(std::vector<eis::DataPoint>& a, const std::vector<eis::DataPoint>& b);

	/**
	* @brief Resamples the data to the given amount of points.
	*
	* @param data Data to resample.
	* @param outputSize Size to resample to.
	* @return The resampled data.
	*/
	std::vector<eis::DataPoint> rescale(const std::vector<eis::DataPoint>& data, size_t outputSize);

	/**
	* @brief Calculates the Pearson correlation between the imaginary and the real part of the data.
	*
	* @param data Data to calculate the Pearson correlation on.
	* @return the Pearson correlation coefficient.
	*/
	fvalue pearsonCorrelation(const std::vector<eis::DataPoint>& data);

	/**
	* @brief Calculates the variance of the distance of the data from a centroid in the nyquist plane.
	*
	* @param data The data to calculate on.
	* @param centroid The centroid to use, if nullptr is passed here, the mean of the data will be used as the centroid.
	* @return The variance.
	*/
	fvalue nyquistAreaVariance(const std::vector<eis::DataPoint>& data, eis::DataPoint* centroid = nullptr);

	/**
	* @brief Finds the maximum distance between subsequent points in the data in the nyquist plane.
	*
	* @param data The data to use.
	* @return The largest distance.
	*/
	fvalue maximumNyquistJump(const std::vector<eis::DataPoint>& data);

	/**
	* @brief Adds white noise to the data.
	*
	* @param data The data to add noise to.
	* @param amplitude The amplitude of the noise.
	* @param relative If true, the amplitude will be taken as relative to the magnitude of the data, otherwise it will be taken as an absolute value.
	*/
	void noise(std::vector<eis::DataPoint>& data, double amplitude, bool relative);

	/**
	* @brief Removes duplicate data points from the data.
	*
	* Duplicates are detected by checking the value of omega for sameness using fvalueEq with the default ulp.
	*
	* @param data the data to remove duplicates from
	*/
	void removeDuplicates(std::vector<eis::DataPoint>& data);

	/**
	* @brief Checks two fvalues for equality.
	*
	* Equality is considered given when the values are within ulp epsilons at the magnitude of the sum of operands.
	*
	* @param a the first value to compare
	* @param b the second value to compare
	* @return true if equal, false if unequal
	*/
	bool fvalueEq(fvalue a, fvalue b, unsigned int ulp = 4);

	/**
	* @brief This function resamples, extrapolates and interpolates to fit the data given to the frequencies also given.
	*
	* Data is resampled to the target size, interpolation for data points is performed using linear interpolation,
	* extrapolation is performed using linear or base 10 logarithmic extrapolation.
	*
	* @param omegas The frequencies to resample the data to.
	* @param data The data to resample.
	* @param linearExtrapolation true if linear extrapolation is to be performed, otherwise base 10 logarithmic extrapolation is used.
	* @return The resampled data.
	*/
	std::vector<eis::DataPoint> fitToFrequencies(std::vector<fvalue> omegas,
	                                             const std::vector<eis::DataPoint>& data,
	                                             bool linearExtrapolation = false);

	/** @} */
}


