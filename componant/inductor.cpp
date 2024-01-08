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

#include "componant/inductor.h"
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

Inductor::Inductor(std::string paramStr, size_t count, bool defaultToRange)
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

std::vector<eis::Range> Inductor::getDefaultParameters(bool range) const
{
	std::vector<eis::Range> out;

	if(range)
		out.push_back(Range(1e-10, 1e-4, 2, true));
	else
		out.push_back(Range(1e-6, 1e-6, 1));

	assert(out.size() == paramCount());
	return out;
}

std::complex<fvalue> Inductor::execute(fvalue omega)
{
	assert(ranges.size() == paramCount());
	return std::complex<fvalue>(0, ranges[0][ranges[0].step]*omega);
}

size_t Inductor::paramCount() const
{
	return 1;
}

char Inductor::getComponantChar() const
{
	return staticGetComponantChar();
}

std::string Inductor::getCode(std::vector<std::string>& parameters)
{
	parameters.push_back(getUniqueName() + "_0");
	std::string N = parameters.back() + "*omega";
	std::string out = "std::complex<fvalue>(0, " + N + ")";
	return out;
}

std::string Inductor::getTorchScript(std::vector<std::string>& parameters)
{
	parameters.push_back(getUniqueName() + "_0");
	std::string out = parameters.back() + "*omegas*1j";
	return out;
}
