#pragma once
#include <string>
#include <vector>
#include <sstream>

std::vector<std::string> tokenize(const std::string& str, const char delim = ' ', const char ignBracketStart = '\0',
								  const char ignBracketEnd = '\0', const char escapeChar = '\0');

size_t opposingBraket(const std::string& str, size_t index, char bracketChar = ')');

size_t deepestBraket(const std::string& str, char bracketChar = ')');
