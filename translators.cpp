#include "translators.h"
#include <vector>
#include <iostream>

struct pair
{
	const char* a;
	const char* b;
};

const struct pair eisRelaxisTable[] =
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

const struct pair eisCdcTable[] =
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

static std::string translateElement(const std::string& in, const struct pair* table, bool reverse = false)
{
	size_t i = 0;
	while(!reverse ? table[i].a : table[i].b)
	{
		if(std::string(!reverse ? table[i].a : table[i].b) == in)
			return std::string(reverse ? table[i].a : table[i].b);
		++i;
	}

	return std::string("x");
}

static bool isValidSymbol(const std::string& in, const struct pair* table, bool reverse = false)
{
	size_t i = 0;
	while(!reverse ? table[i].a : table[i].b)
	{
		if(std::string(!reverse ? table[i].a : table[i].b) == in)
			return true;
		++i;
	}
	return false;
}

static void purgeBrackets(std::string& in)
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

std::string relaxisToEis(const std::string& in)
{
	std::string out;
	std::string work(in);
	purgeBrackets(work);
	for(size_t i = 0; i < work.size(); ++i)
	{
		if(isValidSymbol(std::string(1, work[i]), eisRelaxisTable, true))
		{
			out.append(translateElement(std::string(1, work[i]), eisRelaxisTable, true));

			if(i+1 < work.size() && (i == 0 || work[i-1] != '(' || work[i-1] != ')'))
				out.push_back('-');
		}
	}

	for(size_t i = 0; i < out.size(); ++i)
	{
		if(out[i] == '(' && i < out.size()-2 && out[i] == ')' )
		{
			out.erase(out.begin()+i+2);
			out.erase(out.begin()+i);
			--i;
		}
	}
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

static size_t eisRemoveUnneededBrackets(std::string& in, long int bracketStart = -1)
{
	bool bracketNeeded = false;
	for(size_t i = (bracketStart > 0 ? bracketStart+1 : 0); i < in.size(); ++i)
	{
		if(in[i] == '-')
			bracketNeeded = true;
		if(in[i] == '(')
		{
			i = eisRemoveUnneededBrackets(in, i);
		}
		else if(in[i] == ')')
		{
			if(!bracketNeeded && bracketStart > 0)
			{
				in.erase(in.begin()+i);
				in.erase(in.begin()+bracketStart);
				return i-2;
			}
			return i;
		}
	}
	return in.size()-1;
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
	purgeBrackets(workString);

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

