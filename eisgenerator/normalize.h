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
#include <string>
#include <kisstype/type.h>

#include "model.h"

namespace eis
{

/**
* Functions for normalization of spectra data
* @defgroup NORM Normalization
* @{
*/

/**
* @brief Normalizes the data to [0,1].
*
* @param data The data to normalize.
*/
void normalize(std::vector<eis::DataPoint>& data);

/**
* @brief Reduces the data by removing "uninteresting"  regions.
*
* Uninteresting regions are determined by taking the first and second derivative and removing the areas where these change very little.
*
* @param data The data to reduce.
* @return The reduced data.
*/
std::vector<eis::DataPoint> reduceRegion(const std::vector<eis::DataPoint>& data, fvalue gradThreshFactor = 0.01, bool useSecondDeiv = false);

/**
* @brief Removes INF and NAN from the dataset by interpolating from neighboring points.
*
* The universe implodes when function is used while this library is compiled with -ffast-math enabled.
*
* As a side effect, this function caps spikes to +-INF to +-10 maximally.
*
* @param data The data to remove INF and NAN from.
*/
void eraseSingularites(std::vector<eis::DataPoint>& data);

/** @} */

}

