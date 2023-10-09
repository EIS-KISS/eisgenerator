#include "resistor.h"
#include "strops.h"
#include <vector>
#include <math.h>
#include <cassert>

#include "log.h"

using namespace eis;

Resistor::Resistor(fvalue r)
{
	ranges.clear();
	ranges.push_back(Range(r, r, 1));
}

Resistor::Resistor(std::string paramStr, size_t count, bool defaultToRange)
{
	if(!paramStr.empty())
		ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		ranges.clear();
		if(defaultToRange)
			ranges.push_back(Range(1, 1e4, count, true));
		else
			ranges.push_back(Range(100, 100, 1));
		Log(Log::WARN)<<__func__<<" default range of "<<getComponantString(false)<<" will be used";
	}
}

std::complex<fvalue> Resistor::execute(fvalue omega)
{
	(void)omega;
	assert(ranges.size() == paramCount());
	return std::complex<fvalue>(ranges[0].stepValue(), 0);
}

size_t Resistor::paramCount()
{
	return 1;
}

char Resistor::getComponantChar() const
{
	return staticGetComponantChar();
}

std::string Resistor::getCode(std::vector<std::string>& parameters)
{
	parameters.push_back(getUniqueName() + "_0");
	std::string out = "std::complex<fvalue>(" + parameters.back() + ", 0)";
	return out;
}

std::string Resistor::getTorchScript(std::vector<std::string>& parameters)
{
	parameters.push_back(getUniqueName() + "_0");

	return parameters.back() + "*(1+0j)";
}
