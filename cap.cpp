#include "cap.h"
#include "strops.h"
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

Cap::Cap(std::string paramStr, size_t count)
{
	ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		Log(Log::WARN)<<"invalid parameter string "<<paramStr<<" given to "<<__func__<<", will not be applied\n";
		ranges.clear();
		ranges.push_back(Range(1e-6, 1e-6, 1));
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
