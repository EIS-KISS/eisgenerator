#include "strops.h"

std::vector<std::string> tokenize(const std::string& str, const char delim, const char ignBracketStart, const char ignBracketEnd, const char escapeChar)
{
	std::vector<std::string> tokens;

	std::string token;
	size_t bracketCounter = 0;
	for(size_t i = 0; i < str.size(); ++i)
	{
		if(str[i] == delim && bracketCounter == 0 && (i == 0 || str[i-1] != escapeChar))
		{
			tokens.push_back(token);
			token.clear();
		}
		else
		{
			token.push_back(str[i]);
		}

		if(ignBracketStart == str[i])
			++bracketCounter;
		else if(ignBracketEnd == str[i])
			--bracketCounter;
	}
	if(bracketCounter == 0)
		tokens.push_back(token);
	return tokens;
}
