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
#include <string>
#include <exception>
#include <vector>


namespace eis
{

/**
* Eis model string translators to translate between different description strings used by various 3rd parties
* @defgroup TRANS Translators
* @{
*/

/**
* @brief Translates a model string for RHD instruments RelaxIS to the format used by eisgenerator,
*
* @throws parse_errror if the given string contains syntax errors
* @param in The RelaxIS model string.
* @param params an optional vector of the parameters of the circuit elements
* @return the eisgenerator model string
*/
std::string relaxisToEis(const std::string& in, const std::vector<double>& params = std::vector<double>());

/**
* @brief Translates a string for this library to the format used by RHD instruments RelaxIS
*
* Note that any parameters embedded into the eisgenerator model string will be striped and
* not included in the RelaxIS model.
*
* @throws parse_errror if the given string contains syntax errors
* @param in The eisgenerator model string.
* @return The RelaxIS model string.
*/
std::string eisToRelaxis(const std::string& in);

/**
* @brief Translates a Boukamp Circuit Description Code (CDC) string into the format used by this library
*
* @throws parse_errror if the given string contains syntax errors
* @param in The CDC string.
* @return The eisgenerator model string.
*/
std::string cdcToEis(std::string in);

/**
* @brief Translates a string for this library a Boukamp Circuit Description Code (CDC).
*
* Note that any parameters embedded into the eisgenerator model string will be striped and
* not included in the CDC.
*
* @throws parse_errror if the given string contains syntax errors
* @param in The eisgenerator model string.
* @return The CDC string.
*/
std::string eisToCdc(const std::string& in);

/**
* @brief Translates a model description string as used by the MADAP library into the format used by this library.

* @param in A model description string as used by the MADAP library.
* @param parameters Optionally a string containing the parameters of the model in MADAP parameter format.
* @return The eisgenerator model string optionally including embedded parameters.
*/
std::string madapToEis(const std::string& in, const std::string& parameters = "");

/**
* @brief Translates a string for this library a into the format used by the MADAP library.
*
* Note that any parameters embedded into the eisgenerator model string will be striped and
* not included in the MADAP string.
*
* @throws parse_errror If the given string contains syntax errors.
* @param in The eisgenerator model string.
* @return The MADAP string.
*/
std::string eisToMadap(std::string in);

/**
* @brief Given a eisgenerator model description string, this function will purge all the embedded parameters.
*
* @param in An eisgenerator model string.
*/
void purgeEisParamBrackets(std::string& in);

/** @} */

}
