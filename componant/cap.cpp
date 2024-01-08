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

#include "componant/cap.h"
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

Cap::Cap(std::string paramStr, size_t count, bool defaultToRange)
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

std::vector<eis::Range> Cap::getDefaultParameters(bool range) const
{
	std::vector<eis::Range> out;

	if(range)
		out.push_back(Range(1e-10, 1e-4, 2, true));
	else
		out.push_back(Range(1e-6, 1e-6, 1));

	assert(out.size() == paramCount());
	return out;
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

size_t Cap::paramCount() const
{
	return 1;
}

std::string Cap::getCode(std::vector<std::string>& parameters)
{
	std::string firstParameter = getUniqueName() + "_0";
	parameters.push_back(firstParameter);
	std::string real = "0";
	std::string imag = "0.0-(1.0/(" + firstParameter + "*omega))";
	std::string out = "std::complex<fvalue>(" + real + ", " + imag + ")";
	return out;
}

std::string Cap::getTorchScript(std::vector<std::string>& parameters)
{
	parameters.push_back(getUniqueName() + "_0");

	std::string N = "1/(" + parameters.back() + "*omegas)";
	std::string out =  "0-" + N + "*1j";
	return out;
}
