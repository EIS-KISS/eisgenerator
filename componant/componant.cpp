#include "componant/componant.h"
#include <assert.h>
#include <sstream>
#include "componant/paralellseriel.h"
#include "componant/resistor.h"
#include "componant/cap.h"
#include "componant/constantphase.h"
#include "componant/warburg.h"
#include "log.h"
#include "componant/inductor.h"
#include "componant/finitetr.h"
#include "randomgen.h"

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

std::vector<eis::Range> Componant::getDefaultRange(bool range) const
{
	(void)range;
	assert(paramCount() == 0);
	return std::vector<eis::Range>();
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

std::string Componant::getUniqueName()
{
	if(uniqueName.empty())
	{
		uniqueName.push_back(getComponantChar());
		for(size_t i = 0; i < 3; ++i)
		{
			char ch = static_cast<char>(rd::rand(122-65)+65);
			if(ch > 90 && ch < 97)
				ch = ch + 6;
			uniqueName.push_back(ch);
		}
	}
	return uniqueName;
}

bool Componant::compileable()
{
	std::vector<std::string> parameters;
	return !getCode(parameters).empty();
}

std::string Componant::getCode(std::vector<std::string>& parameters)
{
	(void)parameters;
	return std::string();
}

std::string Componant::getTorchScript(std::vector<std::string>& parameters)
{
	(void)parameters;
	return std::string();
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

Componant* Componant::createNewComponant(char componant, std::string paramStr, size_t count, bool defaultToRange)
{
	switch(componant)
	{
		case Cap::staticGetComponantChar():
			return new Cap(paramStr, count, defaultToRange);
		case Resistor::staticGetComponantChar():
			return new Resistor(paramStr, count, defaultToRange);
		case Inductor::staticGetComponantChar():
			return new Inductor(paramStr, count, defaultToRange);
		case Cpe::staticGetComponantChar():
			return new Cpe(paramStr, count, defaultToRange);
		case Warburg::staticGetComponantChar():
			return new Warburg(paramStr, count, defaultToRange);
		case FiniteTransmitionline::staticGetComponantChar():
			return new FiniteTransmitionline(paramStr, count, defaultToRange);
		default:
			return nullptr;
	}
}

bool Componant::isValidComponantChar(char componantCh)
{
	switch(componantCh)
	{
		case Cap::staticGetComponantChar():
			return true;
		case Resistor::staticGetComponantChar():
			return true;
		case Inductor::staticGetComponantChar():
			return true;
		case Cpe::staticGetComponantChar():
			return true;
		case Warburg::staticGetComponantChar():
			return true;
		case FiniteTransmitionline::staticGetComponantChar():
			return true;
		default:
			return false;
	}
}
