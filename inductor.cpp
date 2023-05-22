#include "inductor.h"
#include "strops.h"
#include <cstdlib>
#include <math.h>
#include <cassert>

#include "log.h"

using namespace eis;

Inductor::Inductor(fvalue L)
{
	ranges.clear();
	ranges.push_back(Range(L, L, 1));
}

Inductor::Inductor(std::string paramStr, size_t count, bool defaultToRange)
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

std::complex<fvalue> Inductor::execute(fvalue omega)
{
	assert(ranges.size() == paramCount());
	return std::complex<fvalue>(0, ranges[0][ranges[0].step]*omega);
}

size_t Inductor::paramCount()
{
	return 1;
}

char Inductor::getComponantChar() const
{
	return staticGetComponantChar();
}

std::string Inductor::getCode(std::vector<std::string>& parameters)
{
	parameters.push_back(getUniqueName() + "_0");
	std::string N = parameters.back() + "*omega";
	std::string out = "std::complex<fvalue>(0, " + N + ")";
	return out;
}
