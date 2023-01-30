#include "finitetr.h"
#include <cstdlib>
#include <math.h>

#include "eisgenerator/componant.h"
#include "eisgenerator/paralellseriel.h"
#include "eisgenerator/resistor.h"
#include "paralellseriel.h"
#include "cap.h"
#include "resistor.h"
#include "log.h"
#include "strops.h"

using namespace eis;

FiniteTransmitionline::FiniteTransmitionline(fvalue c, fvalue r, unsigned int n): _C(c), _R(r), _n(n)
{
	if(n < 1)
	{
		Log(Log::WARN)<<__func__<<" n must be > 0 setting n to 4";
		_n = 4;
	}

	ranges.clear();
	ranges.push_back(Range(_C, _C, 1));
	ranges.push_back(Range(_R, _R, 1));
	ranges.push_back(Range(_n, _n, 1));

	subComponant = createTransmitionLine(_C, _R, _n);
}

void FiniteTransmitionline::setDefaultParam(size_t count, bool defaultToRange)
{
	_C = 1e-6;
	_R = 1000;
	_n = 4;

	ranges.clear();
	if(defaultToRange)
	{
		ranges.push_back(Range(1e-10, 1e-4, count, true));
		ranges.push_back(Range(1, 1e4, count, true));
		ranges.push_back(Range(_n, _n, 1));
	}
	else
	{
		ranges.push_back(Range(_C, _C, 1));
		ranges.push_back(Range(_R, _R, 1));
		ranges.push_back(Range(_n, _n, 1));
	}
}

FiniteTransmitionline::FiniteTransmitionline(std::string paramStr, size_t count, bool defaultToRange)
{
	ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		setDefaultParam(count, defaultToRange);
		Log(Log::WARN)<<__func__<<" default range of "<<getComponantString(false)<<" will be used";
	}

	updateValues();
	if(!subComponant)
		subComponant = createTransmitionLine(_C, _R, _n);
}

FiniteTransmitionline::FiniteTransmitionline(const FiniteTransmitionline& in)
{
	ranges = in.ranges;
	_R = in._R;
	_C = in._C;
	_n = in._n;
	if(subComponant)
		delete subComponant;
	subComponant = createTransmitionLine(_C, _R, _n);
}

std::complex<fvalue> FiniteTransmitionline::execute(fvalue omega)
{
	if(subComponant)
	{
		updateValues();
		return subComponant->execute(omega);
	}
	else
	{
		Log(Log::WARN)<<"Invalid transmitionline used";
		return std::complex<fvalue>(1, 0);
	}
}

char FiniteTransmitionline::getComponantChar() const
{
	return FiniteTransmitionline::staticGetComponantChar();
}

size_t FiniteTransmitionline::paramCount()
{
	return 3;
}

Componant* FiniteTransmitionline::createTransmitionLine(fvalue c, fvalue r, unsigned int n)
{
	Parallel* par = new Parallel;
	par->componants.push_back(new Cap(c));
	par->componants.push_back(new Resistor(r));

	for(unsigned int i = 0; i < n; ++i)
	{
		Serial* ser = new Serial;
		ser->componants.push_back(new Resistor(r));
		ser->componants.push_back(par);

		par = new Parallel;
		par->componants.push_back(ser);
		par->componants.push_back(new Cap(c));
	}

	return par;
}

FiniteTransmitionline::~FiniteTransmitionline()
{
	if(subComponant)
		delete subComponant;
}

void FiniteTransmitionline::updateValues()
{
	assert(ranges.size() == paramCount());
	if(ranges[0].stepValue() != _C || ranges[1].stepValue() != _R || static_cast<size_t>(ranges[2].stepValue()) != _n)
	{
		_C = ranges[0].stepValue();
		_R = ranges[1].stepValue();
		_n = ranges[2].stepValue();
		if(subComponant)
			delete subComponant;
		subComponant = createTransmitionLine(_C, _R, _n);
	}
}
