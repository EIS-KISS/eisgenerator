//
// eisgenerator - a shared libary and application to generate EIS spectra
// Copyright (C) 2022-2024 Carl Philipp Klemm <carl@uvos.xyz>
//
// This file is part of eisgenerator.
//
// eisgenerator is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eisgenerator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with eisgenerator.  If not, see <http://www.gnu.org/licenses/>.
//

#include "translators.h"
#include <vector>
#include <iostream>
#include <cassert>
#include <map>
#include <algorithm>
#include <sstream>

#include "strops.h"
#include "eistype.h"
#include "log.h"
#include "componant/componant.h"

namespace eis
{

const struct std::pair<const char*, const char*> eisRelaxisTable[] =
{
	{"r", "R"},
	{"R", "R"},
	{"c", "C"},
	{"C", "C"},
	{"l", "I"},
	{"L", "I"},
	{"p", "P"},
	{"P", "P"},
	{"w", "W"},
	{"W", "W"},
	{"t", "Tlmo"},
	{"T", "Tlmo"},
	{nullptr, nullptr}
};

const struct std::pair<const char*, const char*> eisCdcTable[] =
{
	{"r", "R"},
	{"R", "R"},
	{"c", "C"},
	{"C", "C"},
	{"l", "L"},
	{"L", "L"},
	{"p", "P"},
	{"P", "P"},
	{"w", "W"},
	{"W", "W"},
	{"t", "T"},
	{"T", "T"},
	{nullptr, nullptr}
};

const struct std::pair<const char*, const char*> eisMadapTable[] =
{
	{"r", "R"},
	{"R", "R"},
	{"c", "C"},
	{"C", "C"},
	{"l", "L"},
	{"L", "L"},
	{"p", "CPE"},
	{"P", "CPE"},
	{"w", "W"},
	{"W", "W"},
	{"w", "Wo"},
	{"W", "Wo"},
	{nullptr, nullptr}
};

static std::string translateElement(const std::string& in, const struct std::pair<const char*, const char*>* table, bool reverse = false)
{
	size_t i = 0;
	while(!reverse ? table[i].first : table[i].second)
	{
		if(std::string(!reverse ? table[i].first : table[i].second) == in)
			return std::string(reverse ? table[i].first : table[i].second);
		++i;
	}

	return std::string("x");
}

static bool isValidSymbol(const std::string& in, const struct std::pair<const char*, const char*>* table, bool reverse = false)
{
	size_t i = 0;
	while(!reverse ? table[i].first : table[i].second)
	{
		if(std::string(!reverse ? table[i].first : table[i].second) == in)
			return true;
		++i;
	}
	return false;
}

void purgeEisParamBrackets(std::string& in)
{
	int bracketCounter = 0;

	std::string out;

	for(size_t i = 0; i < in.size(); ++i)
	{
		if(in[i] == '{' || in[i] == '[' || in[i] == '<')
			++bracketCounter;
		if(bracketCounter == 0)
			out.push_back(in[i]);
		if(in[i] == '}' || in[i] == ']' || in[i] == '>')
			--bracketCounter;
	}
	in = out;
}

std::string relaxisToEis(const std::string& in, const std::vector<double>& params)
{
	std::string out;
	std::string work(in);
	purgeEisParamBrackets(work);

	for(size_t i = 0; i < work.size(); ++i)
	{
		if(isValidSymbol(std::string(1, work[i]), eisRelaxisTable, true))
		{
			if(i != 0 && work[i-1] != '(' && work[i-1] != '-')
			{
				work.insert(i, "-");
				++i;
			}
			if(i < work.size()-1 && work[i+1] != ')' && work[i+1] != '-')
				work.insert(i+1, "-");
		}
	}

	for(size_t i = 0; i < work.size(); ++i)
	{
		if(isValidSymbol(std::string(1, work[i]), eisRelaxisTable, true))
			out.append(translateElement(std::string(1, work[i]), eisRelaxisTable, true));
		else if(work[i] == '-' || work[i] == '(' || work[i] == ')')
			out.push_back(work[i]);
		else
			throw parse_errror("invalid or unkown symbol in relaxis circuit string: " + std::string(1, work[i]));
	}

	for(size_t i = 0; i < out.size(); ++i)
	{
		if(out[i] == '(' && i < out.size()-2 && out[i+2] == ')' )
		{
			out.erase(out.begin()+i+2);
			out.erase(out.begin()+i);
			--i;
		}
	}

	if(!params.empty())
	{
		size_t elementIndex = 0;
		for(size_t i = 0; i < out.size() && elementIndex < params.size(); ++i)
		{
			if(isValidSymbol(std::string(1, out[i]), eisRelaxisTable, false))
			{
				Log::Level oldLevel = Log::level;
				Log::level =  Log::ERROR;
				Componant* componant = Componant::createNewComponant(out[i]);
				Log::level =  oldLevel;
				if(componant->paramCount() > 0)
				{
					std::stringstream paramstream;
					paramstream<<'{'<<std::scientific;
					for(size_t j = 0; j < componant->paramCount() && elementIndex < params.size(); ++j)
					{
						paramstream<<params[elementIndex];
						paramstream<<", ";
						++elementIndex;
					}
					std::string paramStr = paramstream.str();
					paramStr.pop_back();
					paramStr.pop_back();
					paramStr.push_back('}');
					out.insert(i+1, paramStr);
				}
			}
		}
	}

	if(out.size() < 3)
		return out;

	return out;
}

std::string eisToRelaxis(const std::string& in)
{
	std::string work = eisToCdc(in);
	std::string out;

	size_t level = 0;

	for(size_t i = 0; i < work.size(); ++i)
	{
		if(work[i] == '[' || work[i] == '(')
		{
			out.push_back('(');
			++level;
		}
		else if(work[i] == ']' || work[i] == ')')
		{
			if(out.back() == '-')
				out.pop_back();
			out.push_back(')');
			--level;
		}
		else
		{
			std::string eisElement = translateElement(std::string(1, work[i]), eisCdcTable, true);
			std::string relaxisElement = translateElement(eisElement, eisRelaxisTable, false);
			if(level % 2 == 1)
			{
				relaxisElement.push_back(')');
				relaxisElement.insert(relaxisElement.begin(), '(');
			}
			out.append(relaxisElement);
			if(level % 2 == 0)
				out.push_back('-');
		}
	}

	if(out.back() == '-')
		out.pop_back();

	return out;
}

std::string cdcToEis(std::string in)
{
	std::string out;

	size_t level = 0;

	for(size_t i = 0; i < in.size(); ++i)
	{
		if(in[i] == '[' || in[i] == '(')
		{
			out.push_back('(');
			++level;
		}
		else if(in[i] == ']' || in[i] == ')')
		{
			if(out.back() == '-')
				out.pop_back();
			out.push_back(')');
			--level;
		}
		else
		{
			out.append(translateElement(std::string(1, in[i]), eisCdcTable, true));
			if(level % 2 == 0)
				out.push_back('-');
		}
	}

	if(out.back() == '-')
		out.pop_back();

	eisRemoveUnneededBrackets(out);
	return out;
}

static void balanceBrakets(std::string& in)
{
	std::string appendStr;
	for(size_t i = 0; i < in.size(); ++i)
	{
		switch(in[i])
		{
			case '(':
				appendStr.push_back(')');
				continue;
			case '[':
				appendStr.push_back(']');
				continue;
			case ')':
				appendStr.pop_back();
				continue;
			case ']':
				appendStr.pop_back();
				continue;
			default:
				continue;
		}
	}
	for(size_t i = appendStr.size(); i > 0; --i)
		in.push_back(appendStr[i-1]);
}

static std::string cdcForBacket(const std::string& in, long int bracketStart)
{
	size_t bracketEnd;
	for(bracketEnd = bracketStart+1 ; bracketEnd < in.size() && in[bracketEnd] != ')'; ++bracketEnd);

	std::string bracket = in.substr(bracketStart+1, bracketEnd-(bracketStart+1));

	bool serial = true;

	if(bracket.size() > 1 && bracket[1] != '-')
		serial = false;

	std::string out("[");
	if(!serial)
		out.push_back('(');

	for(size_t i = 0; i < bracket.size(); ++i)
	{
		if(bracket[i] == '-')
			continue;
		if(serial && i+1 < bracket.size() && bracket[i+1] != '-')
		{
			out.push_back('(');
			serial = false;
		}

		if(bracket[i] > 47 && bracket[i] < 58)
			out.push_back(bracket[i]);
		else if(isValidSymbol(std::string(1, bracket[i]), eisCdcTable))
			out.append(translateElement(std::string(1, bracket[i]), eisCdcTable));

		if(!serial && i+1 < bracket.size() && bracket[i+1] == '-' )
		{
			out.push_back(')');
			serial = true;
		}

	}

	balanceBrakets(out);

	return out;
}

static size_t getDeepestBraket(const std::string& in, size_t& maxDepth)
{
	size_t loc = 0;
	size_t depth = 0;
	maxDepth = 0;

	for(size_t i = 0; i < in.size(); ++i)
	{
		if(in[i] == '(')
		{
			++depth;
			if(depth > maxDepth)
			{
				maxDepth = depth;
				loc = i;
			}
		}
		else if(in[i] == ')')
		{
			--depth;
		}
	}
	return loc;
}

static void replaceBraket(std::string& in, size_t start, size_t num)
{
	char opposing;
	switch(in[start])
	{
		case '(':
			opposing = ')';
			break;
		case '[':
			opposing = ']';
			break;
		default:
			return;
	}

	size_t end;
	for(end = start; end < in.size() && in[end] != opposing; ++end);

	in.erase(start, (end-start)+1);
	in.insert(start, std::to_string(num));

}

std::string eisToCdc(const std::string& in)
{
	std::vector<std::string> brackets;
	std::string workString = in;
	purgeEisParamBrackets(workString);

	size_t maxDepth;
	size_t bracketStart = getDeepestBraket(workString, maxDepth);
	while(maxDepth > 0)
	{
		brackets.push_back(cdcForBacket(workString, bracketStart));
		replaceBraket(workString, bracketStart, brackets.size()-1);
		bracketStart = getDeepestBraket(workString, maxDepth);
	}

	std::string out = cdcForBacket(workString, -1);

	for(size_t i = 0; i < out.size(); ++i)
	{
		if(out[i] > 47 && out[i] < 58 && static_cast<size_t>(out[i]-48) < brackets.size())
		{
			out.insert(i+1, brackets[out[i]-48]);
			out.erase(out.begin()+i);
		}
	}

	bool paralelFirst = false;
	for(size_t i = 1; i < out.size(); ++i)
	{
		if(out[i] == '[')
		{
			paralelFirst = true;
			break;
		}
		else if(out[i] == '(')
		{
			break;
		}
	}

	if(!paralelFirst)
	{
		out.erase(out.begin());
		out.erase(out.end()-1);
	}

	return out;
}

static std::string madapStrip(const std::string& in)
{
	std::string out;
	out.reserve(in.size());
	for(char ch : in)
	{
		if(ch == 'p')
			continue;
		out.push_back(ch);
	}
	return out;
}

std::string madapTranslateSerial(const std::string& in, const std::map<std::string, std::string>& parameters);

std::string madapTranslateParalell(const std::string& in, const std::map<std::string, std::string>& parameters)
{
	std::string out;
	out.push_back('(');
	std::vector<std::string> tokens = tokenize(in, ',', '(', ')', '\\');
	for(std::string& token : tokens)
	{
		out.push_back('(');
		if(token[0] == '(')
		{
			if(token.back() != ')')
				throw parse_errror("unbounded bracket");
			token.pop_back();
			token.erase(token.begin());
			out.append(madapTranslateParalell(token, parameters));
		}
		else
		{
			out.append(madapTranslateSerial(token, parameters));
		}
		out.push_back(')');
	}
	out.push_back(')');
	return out;
}

std::string madapTranslateSerial(const std::string& in, const std::map<std::string, std::string>& parameters)
{
	std::string out;

	std::vector<std::string> tokens = tokenize(in, '-', '(', ')', '\\');
	for(std::string& token : tokens)
	{
		if(token[0] == '(')
		{
			if(token.back() != ')')
				throw parse_errror("unbounded bracket");
			token.pop_back();
			token.erase(token.begin());
			out.append(madapTranslateParalell(token, parameters));
		}
		else
		{
			auto paramIterator = parameters.find(token);
			std::string parameterStr;
			if(paramIterator != parameters.end())
				parameterStr = paramIterator->second;

			if(token.back() >= 48 && token.back() <= 57)
				token.pop_back();

			if(!isValidSymbol(token, eisMadapTable, true))
				throw parse_errror("invalid symbol \"" + token + "\"");
			out.append(translateElement(token, eisMadapTable, true));
			out.append(parameterStr);
		}
		out.push_back('-');
	}

	if(out.back() == '-')
		out.pop_back();

	return out;
}

std::vector<std::string> madapGetFlatComponants(const std::string& in)
{
	std::vector<std::string> out;
	std::vector<std::string> tokens = tokenize(in, '-');
	for(const std::string& token : tokens)
	{
		std::vector<std::string> subtokens = tokenize(token, ',');
		for(std::string& subtoken : subtokens)
		{
			subtoken = stripWhitespace(subtoken);
			if(subtoken.empty())
				continue;
			if(subtoken[0] == '(' || subtoken[0] == ')')
				subtoken.erase(subtoken.begin());
			if(subtoken.back() == '(' || subtoken.back() == ')')
				subtoken.pop_back();
			out.push_back(subtoken);
		}
	}
	return out;
}

std::string madapGenerateEisParamString(const std::vector<double>& paramNums, size_t& currParam, size_t paramCount)
{
	if(paramCount == 0)
		return "";
	if(currParam+paramCount-1 >= paramNums.size())
		throw parse_errror("Not enough parameters");

	std::stringstream out;
	out<<'{'<<std::scientific;

	for(size_t i = 0; i < paramCount; ++i)
	{
		out<<paramNums.at(currParam);
		if(i+1 != paramCount)
			out<<", ";
		++currParam;
	}

	out<<'}';
	return out.str();
}

std::map<std::string, std::string> madapParseParameters(const std::string& in, std::string parameters)
{
	std::map<std::string, std::string> out;

	if(parameters.empty())
		return out;

	parameters = stripWhitespace(parameters);
	if(parameters[0] == '[')
		parameters.erase(parameters.begin());
	if(parameters.back() == ']')
		parameters.pop_back();

	std::vector<double> paramNums;
	std::vector<std::string> tokens = tokenize(parameters, ',', '(', ')', '\\');
	for(const std::string& token : tokens)
	{
		std::vector<std::string> subtokens = tokenize(token, ',');
		if(subtokens[0][0] == '(')
			subtokens[0].erase(subtokens[0].begin());
		if(subtokens[0].back() == ')')
			subtokens[0].pop_back();
		Log(Log::DEBUG)<<__func__<<" Adding param "<<subtokens[0];
		paramNums.push_back(std::stod(subtokens[0]));
	}

	if(paramNums.empty())
		throw parse_errror("invaid parameter string");

	size_t paramCounter = 0;
	std::vector<std::string> componants = madapGetFlatComponants(in);
	for(const std::string& componant : componants)
	{
		std::string workComponant = componant;
		auto newEnd = std::remove_if(workComponant.begin(), workComponant.end(), [](char ch){return ch >= 48 && ch <= 57;});
		workComponant.erase(newEnd, workComponant.end());
		std::string eisComponant = translateElement(workComponant, eisMadapTable, true);
		if(eisComponant == "x")
			throw parse_errror("invalid symbol \"" + componant + "\"");
		if(!out.insert({componant, ""}).second)
			throw parse_errror("duplicate symbol \"" + componant + "\"");

		if(eisComponant == "r")
			out[componant] = madapGenerateEisParamString(paramNums, paramCounter, 1);
		else if(eisComponant == "c")
			out[componant] = madapGenerateEisParamString(paramNums, paramCounter, 1);
		else if(eisComponant == "p")
			out[componant] = madapGenerateEisParamString(paramNums, paramCounter, 2);
		else if(eisComponant == "l")
			out[componant] = madapGenerateEisParamString(paramNums, paramCounter, 1);
		else if(eisComponant == "w")
			out[componant] = madapGenerateEisParamString(paramNums, paramCounter, 1);
	}

	return out;
}

std::string madapToEis(const std::string& in, const std::string& parameters)
{
	if(in.empty())
		return std::string("");
	std::string work = madapStrip(in);
	work = stripWhitespace(work);

	std::map<std::string, std::string> parameterMap = madapParseParameters(work, parameters);

	std::string out = madapTranslateSerial(work, parameterMap);

	eisRemoveUnneededBrackets(out);

	return out;
}

std::string eisToMadap(std::string in)
{
	purgeEisParamBrackets(in);
	Log(Log::ERROR)<<__func__<<" is not implemented";
	assert(false);
	return in;
}

}

