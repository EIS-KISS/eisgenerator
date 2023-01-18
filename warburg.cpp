#include "warburg.h"
#include "strops.h"
#include <cstdlib>
#include <math.h>
#include <cassert>

#include "log.h"

using namespace eis;

Warburg::Warburg(fvalue a)
{
	ranges.clear();
	ranges.push_back(Range(a, a, 1));
}

Warburg::Warburg(std::string paramStr, size_t count, bool defaultToRange)
{
	if(!paramStr.empty())
		ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		ranges.clear();
		if(defaultToRange)
			ranges.push_back(Range(10, 100, count, true));
		else
			ranges.push_back(Range(50, 50, 1));
		Log(Log::WARN)<<__func__<<" default range of "<<getComponantString(false)<<" will be used";
	}
}

std::complex<fvalue> Warburg::execute(fvalue omega)
{
	assert(ranges.size() == paramCount());
	fvalue N = ranges[0][ranges[0].step]/(sqrt(omega));
	return std::complex<fvalue>(N, 0-N);
}

size_t Warburg::paramCount()
{
	return 1;
}

char Warburg::getComponantChar() const
{
	return staticGetComponantChar();
}
