#include "componant/constantphase.h"
#include <cstdlib>
#include <string>
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
	setDefaultParam(1, false);
}

Cpe::Cpe(fvalue q, fvalue alpha)
{
	ranges.clear();
	ranges.push_back(Range(q, q, 1));
	ranges.push_back(Range(alpha, alpha, 1));
}

Cpe::Cpe(std::string paramStr, size_t count, bool defaultToRange)
{
	if(!paramStr.empty())
		ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		setDefaultParam(count, defaultToRange);
		Log(Log::WARN)<<__func__<<" default range of "<<getComponantString(false)<<" will be used";
	}

}

void Cpe::setDefaultParam(size_t count, bool defaultToRange)
{
	ranges.clear();
	if(defaultToRange)
	{
		ranges.push_back(Range(1e-10, 1e-4, count, true));
		ranges.push_back(Range(0.5, 0.9, count));
	}
	else
	{
		ranges.push_back(Range(1e-7, 1e-7, 1));
		ranges.push_back(Range(0.9, 0.9, 1));
	}
}

std::complex<fvalue> Cpe::execute(fvalue omega)
{
	assert(ranges.size() == paramCount());
	fvalue real = (1.0/(ranges[0][ranges[0].step]*std::pow(omega, ranges[1][ranges[1].step])))*std::cos((M_PI/2)*ranges[1][ranges[1].step]);
	fvalue imag = 0-(1.0/(ranges[0][ranges[0].step]*std::pow(omega, ranges[1][ranges[1].step])))*std::sin((M_PI/2)*ranges[1][ranges[1].step]);
	return std::complex<fvalue>(real, imag);
}

size_t Cpe::paramCount()
{
	return 2;
}

char Cpe::getComponantChar() const
{
	return Cpe::staticGetComponantChar();
}

std::string Cpe::getCode(std::vector<std::string>& parameters)
{
	std::string firstParameter = getUniqueName() + "_0";
	std::string secondParameter = getUniqueName() + "_1";
	parameters.push_back(firstParameter);
	parameters.push_back(secondParameter);
	std::string real = "(1.0/(" + firstParameter + "*std::pow(omega," + secondParameter + ")))*std::cos((M_PI/2)*" + secondParameter + ")";
	std::string imag = "0-(1.0/(" + firstParameter + "*std::pow(omega," + secondParameter + ")))*std::sin((M_PI/2)*" + secondParameter + ")";
	std::string out = "std::complex<fvalue>(" + real +", " + imag + ")";
	return out;
}

std::string Cpe::getTorchScript(std::vector<std::string>& parameters)
{
	std::string hpi = std::to_string(M_PI/2);
	std::string firstParameter = getUniqueName() + "_0";
	std::string secondParameter = getUniqueName() + "_1";
	parameters.push_back(firstParameter);
	parameters.push_back(secondParameter);
	std::string real = "(1/(" + firstParameter + "*torch.pow(omegas,"+ secondParameter +")))*torch.cos("+ hpi +"*" + secondParameter + ")";
	std::string imag = "(1/(" + firstParameter + "*torch.pow(omegas,"+ secondParameter +")))*torch.sin("+ hpi +"*" + secondParameter + ")";
	std::string out = real + '-' + imag + "*1j";
	return out;
}
