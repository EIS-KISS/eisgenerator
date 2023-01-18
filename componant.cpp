#include "componant.h"
#include <assert.h>
#include <sstream>
#include "paralellseriel.h"
#include "resistor.h"
#include "cap.h"
#include "constantphase.h"
#include "warburg.h"
#include "log.h"
#include "inductor.h"
#include "finitetr.h"

using namespace eis;

void Componant::setParamRanges(const std::vector<eis::Range>& rangesIn)
{
	ranges = rangesIn;
}

std::vector<eis::Range>& Componant::getParamRanges()
{
	return ranges;
}

std::vector<eis::Range> Componant::getParamRanges() const
{
	return ranges;
}

std::string Componant::getComponantString(bool currentValue) const
{
	std::stringstream out;
	out<<getComponantChar();
	std::vector<eis::Range> ranges = getParamRanges();

	if(ranges.empty())
		return out.str();

	out<<'{';
	for(const eis::Range& range : ranges)
	{
		if(currentValue)
			out<<range.stepValue()<<", ";
		else
			out<<range.getString()<<", ";
	}

	std::string outStr = out.str();
	outStr.pop_back();
	outStr.back() = '}';
	return outStr;
}

Componant* Componant::copy(Componant* componant)
{
	switch(componant->getComponantChar())
	{
		case Resistor::staticGetComponantChar():
			return new Resistor(*dynamic_cast<Resistor*>(componant));
		case Cap::staticGetComponantChar():
			return new Cap(*dynamic_cast<Cap*>(componant));
		case Inductor::staticGetComponantChar():
			return new Inductor(*dynamic_cast<Inductor*>(componant));
		case Cpe::staticGetComponantChar():
			return new Cpe(*dynamic_cast<Cpe*>(componant));
		case Warburg::staticGetComponantChar():
			return new Warburg(*dynamic_cast<Warburg*>(componant));
		case FiniteTransmitionline::staticGetComponantChar():
			return new FiniteTransmitionline(*dynamic_cast<FiniteTransmitionline*>(componant));
		case Parallel::staticGetComponantChar():
			return new Parallel(*dynamic_cast<Parallel*>(componant));
		case Serial::staticGetComponantChar():
			return new Serial(*dynamic_cast<Serial*>(componant));
		default:
			Log(Log::ERROR)<<"unimplmented type copy for "<<componant->getComponantChar();
			assert(0);
			break;
	}
	return nullptr;
}
