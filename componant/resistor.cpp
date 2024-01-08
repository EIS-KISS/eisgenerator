//
// eisgenerator - a shared libary and application to generate EIS spectra
// Copyright (C) 2022-2024 Carl Philipp Klemm <carl@uvos.xyz>
//
// This file is part of eisgenerator.
//
// eisgenerator is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eisgenerator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with eisgenerator.  If not, see <http://www.gnu.org/licenses/>.
//

#include "componant/resistor.h"
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

Resistor::Resistor(std::string paramStr, size_t count, bool defaultToRange)
{
	if(!paramStr.empty())
		ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		ranges = getDefaultParameters(defaultToRange);
		if(defaultToRange)
		{
			for(eis::Range& range : ranges)
				range.count = count;
		}
		Log(Log::WARN)<<__func__<<" default range of "<<getComponantString(false)<<" will be used";
	}
}

std::vector<eis::Range> Resistor::getDefaultParameters(bool range) const
{
	std::vector<eis::Range> out;

	if(range)
		out.push_back(Range(1, 1e4, 2, true));
	else
		out.push_back(Range(100, 100, 1));

	assert(out.size() == paramCount());
	return out;
}

std::complex<fvalue> Resistor::execute(fvalue omega)
{
	(void)omega;
	assert(ranges.size() == paramCount());
	return std::complex<fvalue>(ranges[0].stepValue(), 0);
}

size_t Resistor::paramCount() const
{
	return 1;
}

char Resistor::getComponantChar() const
{
	return staticGetComponantChar();
}

std::string Resistor::getCode(std::vector<std::string>& parameters)
{
	parameters.push_back(getUniqueName() + "_0");
	std::string out = "std::complex<fvalue>(" + parameters.back() + ", 0)";
	return out;
}

std::string Resistor::getTorchScript(std::vector<std::string>& parameters)
{
	parameters.push_back(getUniqueName() + "_0");

	return parameters.back() + "*(1+0j)";
}
