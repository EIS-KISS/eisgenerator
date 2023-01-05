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

Warburg::Warburg(std::string paramStr, size_t count)
{
	ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		Log(Log::WARN)<<"invalid parameter string "<<paramStr<<" given to "<<__func__<<", will not be applied\n";
		ranges.clear();
		ranges.push_back(Range(2e4, 2e4, 1));
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
