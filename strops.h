//SPDX-License-Identifier:         LGPL-3.0-or-later
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
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

std::vector<std::string> tokenize(const std::string& str, const char delim = ' ', const char ignBracketStart = '\0',
								  const char ignBracketEnd = '\0', const char escapeChar = '\0');
std::vector<std::string> tokenizeBinaryIgnore(const std::string& str, const char delim, const char ignoreBraket = '\0',
											  const char escapeChar = '\0');

size_t opposingBraket(const std::string& str, size_t index, char bracketChar = ')');

size_t deepestBraket(const std::string& str, std::string bracketChars = "(", size_t* levelOut = nullptr);

char getOpposingBracketChar(const char ch);

std::string stripWhitespace(const std::string& in);

void stripQuotes(std::string& in);

size_t eisRemoveUnneededBrackets(std::string& in, long int bracketStart = -1);
