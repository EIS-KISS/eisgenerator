/* * eisgenerator - a shared libary and application to generate EIS spectra
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
	Various math functions perfomed on eisgenerator types
	* @defgroup MATH Math
	* @{
	*/

	/**
	* @brief Calculates the element wise absolute gradiant at the given point of the data given
	*
	* @param data the data to calculate the gradiant on
	* @param index the position in the data to calculate the gradiant at
	* @return the element wise absolute gradiant abs(re(grad(a)))+abs(im(grad(a)))i
	*/
	std::complex<fvalue> absGrad(const std::vector<eis::DataPoint>& data, size_t index);

	/**
	* @brief Calculates derivative at the given point of the data given
	*
	* @param data the data to calculate the gradiant on
	* @param index the position in the data to calculate the gradiant at
	* @return the derivative
	*/
	fvalue grad(const std::vector<fvalue>& data, const std::vector<fvalue>& omega, size_t index);

	/**
	* @brief Calculates the gradiant at the given point of the data given
	*
	* @param data the data to calculate the gradiant on
	* @param index the position in the data to calculate the gradiant at
	* @return the gradiant
	*/
	std::complex<fvalue> grad(const std::vector<eis::DataPoint>& data, size_t index);

	/**
	* @brief Calculates the mean of the given data
	*
	* @param data the data to calculate the mean of
	* @return the mean
	*/
	fvalue mean(const std::vector<fvalue>& data);

	/**
	* @brief Calculates the mean of the given data
	*
	* @param data the data to calculate the mean of
	* @return the mean
	*/
	std::complex<fvalue> mean(const std::vector<eis::DataPoint>& data);

	/**
	* @brief Calculates the median of the given data
	*
	* @param data the data to calculate the median of
	* @return the median
	*/
	fvalue median(std::vector<fvalue> data);

	/**
	* @brief Calculates the median of the given data
	*
	* @param data the data to calculate the median of
	* @return the median
	*/
	std::complex<fvalue> median(const std::vector<eis::DataPoint>& data);

	/**
	* @brief Resamples the data to the given amount of points
	*
	* @param data data to resample
	* @param outputSize size to resample to
	* @return the resampled data
	*/
	std::vector<eis::DataPoint> rescale(const std::vector<eis::DataPoint>& data, size_t outputSize);

	/**
	* @brief Calculates the pearson correlation between the imaginary and the real part of the data
	*
	* @param data data caluclate the pearson correlation on
	* @return the pearson correlation coefficent
	*/
	fvalue pearsonCorrelation(const std::vector<eis::DataPoint>& data);

	/**
	* @brief Calculates the variance of the distance of the data from a centroid in the nyquist plane
	*
	* @param data the data to calculate on
	* @param centroid the centroid to use, if nullptr is passed here, the mean of the data will be used as the centroid
	* @return the pearson correlation coefficent
	*/
	fvalue nyquistAreaVariance(const std::vector<eis::DataPoint>& data, eis::DataPoint* centroid = nullptr);

	/**
	* @brief Finds the maximum distance between subsiquent points in the data in the nyquist plane
	*
	* @param data the data to use
	* @return the largest distance
	*/
	fvalue maximumNyquistJump(const std::vector<eis::DataPoint>& data);

	/**
	* @brief Adds white noise to the data
	*
	* @param data the data to add noise to
	* @param amplitude the amplitude of the noise
	* @param relative if true the amplitude will be taken as relative to the magnitude of the data, otherwise it will be taken as an absolute value
	*/
	void noise(std::vector<eis::DataPoint>& data, double amplitude, bool relative);

	/**
	* @brief Removes duplicate data points form the data
	* duplicates are detected by checking the value of omega for samenes using fvalueEq with the default ulp
	*
	* @param data the data to remove duplicates from
	*/
	void removeDuplicates(std::vector<eis::DataPoint>& data);

	/**
	* @brief Checks two fvalues for equality
	*
	* Equality is considered given when the values are within ulp epsilons at the magnitude of the sum of operands.
	*
	* @param a the first value to compeare
	* @param b the second value to compeare
	* @return true if equal, false if unequal
	*/
	bool fvalueEq(fvalue a, fvalue b, unsigned int ulp = 4);

	/**
	* @brief this function resamples, extrapolates and intrapolates to fit the data given to the frequencies also given
	*
	* Data is resampled to the target size, iterpolation for datapoints is performed using linear interpolation
	* extrapolation is performed using linear or base 10 logarithmic extrapolation.
	*
	* @param omegas the frequencies to resample the data to
	* @param data the data to resample
	* @param linearExtrapolation true if linear extrapolation is to be perfomed, otherwise base 10 logarithmic extrapolation is used
	* @return the resampled data
	*/
	std::vector<eis::DataPoint> fitToFrequencies(std::vector<fvalue> omegas,
	                                             const std::vector<eis::DataPoint>& data,
	                                             bool linearExtrapolation = false);

	/** @} */
}


