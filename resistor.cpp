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

Resistor::Resistor(std::string paramStr, size_t count)
{
	ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		Log(Log::WARN)<<"invalid parameter string "<<paramStr<<" given to "<<__func__<<", will not be applied\n";
		ranges.clear();
		ranges.push_back(Range(1e3, 1e3, 1));
	}
}

std::complex<fvalue> Resistor::execute(fvalue omega)
{
	(void)omega;
	assert(ranges.size() == paramCount());
	return std::complex<fvalue>(ranges[0][ranges[0].step], 0);
}

size_t Resistor::paramCount()
{
	return 1;
}

char Resistor::getComponantChar() const
{
	return staticGetComponantChar();
}
