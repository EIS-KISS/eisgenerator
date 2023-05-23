#include "strops.h"
#include <algorithm>
#include <cassert>

#include "log.h"

char getOpposingBracketChar(const char ch)
{
	switch(ch)
	{
		case '(':
			return ')';
		case ')':
			return '(';
		case '{':
			return '}';
		case '}':
			return '{';
		case '[':
			return ']';
		case ']':
			return '[';
		case '<':
			return '>';
		default:
			eis::Log(eis::Log::ERROR)<<ch<<" is not a valid bracket char";
			assert(false);
	}
	return 'x';
}

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

std::vector<std::string> tokenizeBinaryIgnore(const std::string& str, const char delim, const char ignoreBraket, const char escapeChar)
{
	std::vector<std::string> tokens;

	std::string token;
	bool inBaracket = false;
	for(size_t i = 0; i < str.size(); ++i)
	{
		if(str[i] == delim && !inBaracket && (i == 0 || str[i-1] != escapeChar))
		{
			tokens.push_back(token);
			token.clear();
		}
		else
		{
			token.push_back(str[i]);
		}

		if(ignoreBraket == str[i])
			inBaracket = !inBaracket;
	}
	if(!inBaracket)
		tokens.push_back(token);
	return tokens;
}

size_t opposingBraket(const std::string& str, size_t index, char bracketChar)
{
	for(size_t i = index; i < str.size(); ++i)
	{
		if(str[i] == bracketChar)
			return i;
	}
	return std::string::npos;
}

size_t deepestBraket(const std::string& str, std::string bracketChars, size_t* levelOut)
{
	size_t deepestPos = std::string::npos;
	size_t deepestLevel = 0;
	size_t level = 0;

	std::string opposingBraketChars;

	for(char ch : bracketChars)
		opposingBraketChars.push_back(getOpposingBracketChar(ch));

	for(size_t i = 0; i < str.size(); ++i)
	{
		if(std::find(bracketChars.begin(), bracketChars.end(), str[i]) != bracketChars.end())
		{
			++level;
			if(level > deepestLevel)
			{
				deepestLevel = level;
				deepestPos = i;
			}
		}
		else if(std::find(opposingBraketChars.begin(), opposingBraketChars.end(), str[i]) != opposingBraketChars.end())
		{
			--level;
		}
	}

	if(levelOut)
		*levelOut = deepestLevel;
	return deepestPos;
}

size_t eisRemoveUnneededBrackets(std::string& in, long int bracketStart)
{
	bool bracketNeeded = false;
	size_t paramBracketCount = 0;

	for(size_t i = (bracketStart >= 0 ? bracketStart+1 : 0); i < in.size(); ++i)
	{
		if(paramBracketCount == 0)
		{
			if(in[i] == '-')
				bracketNeeded = true;
			else if(in[i] == '(')
			{
				i = eisRemoveUnneededBrackets(in, i);
			}
			else if(in[i] == ')')
			{
				if(!bracketNeeded && bracketStart >= 0)
				{
					in.erase(in.begin()+i);
					in.erase(in.begin()+bracketStart);
					return i-2;
				}
				return i;
			}
		}
		if(in[i] == '{')
			++paramBracketCount;
		else if(in[i] == '}' && paramBracketCount > 0)
			--paramBracketCount;
	}
	return in.size()-1;
}

std::string stripWhitespace(const std::string& in)
{
	std::string out;
	out.reserve(in.size());
	for(char ch : in)
	{
		if(ch <= 32 || ch == 127)
			continue;
		out.push_back(ch);
	}
	return out;
}
