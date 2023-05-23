#pragma once
#include <string>
#include <vector>
#include <sstream>

std::vector<std::string> tokenize(const std::string& str, const char delim = ' ', const char ignBracketStart = '\0',
								  const char ignBracketEnd = '\0', const char escapeChar = '\0');
std::vector<std::string> tokenizeBinaryIgnore(const std::string& str, const char delim, const char ignoreBraket = '\0',
											  const char escapeChar = '\0');

size_t opposingBraket(const std::string& str, size_t index, char bracketChar = ')');

size_t deepestBraket(const std::string& str, std::string bracketChars = "(", size_t* levelOut = nullptr);

char getOpposingBracketChar(const char ch);

std::string stripWhitespace(const std::string& in);

size_t eisRemoveUnneededBrackets(std::string& in, long int bracketStart = -1);
