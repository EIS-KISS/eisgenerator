#include "componant/cap.h"
#include <cstdlib>
#include <math.h>
#include <cassert>

#include "log.h"

using namespace eis;

Cap::Cap(fvalue c)
{
	ranges.clear();
	ranges.push_back(Range(c, c, 1));
}

Cap::Cap(std::string paramStr, size_t count, bool defaultToRange)
{
	if(!paramStr.empty())
		ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		ranges.clear();
		if(defaultToRange)
			ranges.push_back(Range(1e-10, 1e-4, count, true));
		else
			ranges.push_back(Range(1e-6, 1e-6, 1));
		Log(Log::WARN)<<__func__<<" default range of "<<getComponantString(false)<<" will be used";
	}
}

std::complex<fvalue> Cap::execute(fvalue omega)
{
	assert(ranges.size() > 0);
	return std::complex<fvalue>(0, 0.0-(1.0/(ranges[0][ranges[0].step]*omega)));
}

char Cap::getComponantChar() const
{
	return Cap::staticGetComponantChar();
}

size_t Cap::paramCount()
{
	return 1;
}

std::string Cap::getCode(std::vector<std::string>& parameters)
{
	std::string firstParameter = getUniqueName() + "_0";
	parameters.push_back(firstParameter);
	std::string real = "0";
	std::string imag = "0.0-(1.0/(" + firstParameter + "*omega))";
	std::string out = "std::complex<fvalue>(" + real + ", " + imag + ")";
	return out;
}

std::string Cap::getTorchScript(std::vector<std::string>& parameters)
{
	parameters.push_back(getUniqueName() + "_0");

	std::string N = "1/(" + parameters.back() + "*omegas)";
	std::string out =  "0-" + N + "*1j";
	return out;
}
