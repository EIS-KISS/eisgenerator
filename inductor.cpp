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

Inductor::Inductor(std::string paramStr, size_t count)
{
	ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		Log(Log::WARN)<<"invalid parameter string "<<paramStr<<" given to "<<__func__<<", will not be applied\n";
		ranges.clear();
		ranges.push_back(Range(1e-6, 1e-6, 1));
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
