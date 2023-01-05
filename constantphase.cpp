#include "constantphase.h"
#include "strops.h"
#include <cstdlib>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cassert>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#include "log.h"

using namespace eis;

Cpe::Cpe()
{
	setDefaultParam();
}

Cpe::Cpe(fvalue q, fvalue alpha)
{
	ranges.clear();
	ranges.push_back(Range(q, q, 1));
	ranges.push_back(Range(alpha, alpha, 1));
}

Cpe::Cpe(std::string paramStr, size_t count)
{
	ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		Log(Log::WARN)<<"invalid parameter string "<<paramStr<<" given to "<<__func__<<", will not be applied\n";
		setDefaultParam();
	}
}

void Cpe::setDefaultParam()
{
	ranges.clear();
	ranges.push_back(Range(1e-7, 1e-7, 1));
	ranges.push_back(Range(0.9, 0.9, 1));
}

std::complex<fvalue> Cpe::execute(fvalue omega)
{
	assert(ranges.size() == paramCount());
	return std::complex<fvalue>((1.0/(ranges[0][ranges[0].step]*pow(omega, ranges[1][ranges[1].step])))*cos((M_PI/2)*ranges[1][ranges[1].step]),
	                           0-(1.0/(ranges[0][ranges[0].step]*pow(omega, ranges[1][ranges[1].step])))*sin((M_PI/2)*ranges[1][ranges[1].step]));
}

size_t Cpe::paramCount()
{
	return 2;
}

char Cpe::getComponantChar() const
{
	return Cpe::staticGetComponantChar();
}
