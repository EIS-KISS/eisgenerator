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

#include "componant/trc.h"
#include <cstdlib>
#include <math.h>

#include "log.h"

using namespace eis;

TransmissionLineClosed::TransmissionLineClosed(fvalue r, fvalue q, fvalue a, fvalue l)
{
	ranges.clear();
	ranges.reserve(4);
	ranges.push_back(Range(r, r, 1));
	ranges.push_back(Range(q, q, 1));
	ranges.push_back(Range(a, a, 1));
	ranges.push_back(Range(l, l, 1));
}

TransmissionLineClosed::TransmissionLineClosed(std::string paramStr, size_t count, bool defaultToRange)
{
	if(!paramStr.empty())
		ranges = Range::rangesFromParamString(paramStr, count);

	if(ranges.size() != paramCount())
	{
		ranges = getDefaultParameters(defaultToRange);
		Log(Log::WARN)<<__func__<<" default range of "<<getComponantString(false)<<" will be used";
	}
}

std::vector<eis::Range> TransmissionLineClosed::getDefaultParameters(bool range) const
{
	std::vector<eis::Range> out;
	out.reserve(4);

	if(range) {
		out.push_back(Range(1, 1e4, 2, true));
		out.push_back(Range(1e-10, 1e-4, 2, true));
		out.push_back(Range(0.15, 1, 2, true));
		out.push_back(Range(0.01, 1000, 2, true));
	}
	else {
		out.push_back(Range(50, 50, 1));
		out.push_back(Range(1e-6, 1e-6, 1));
		out.push_back(Range(1, 1, 1));
		out.push_back(Range(1, 1, 1));
	}

	assert(out.size() == paramCount());
	return out;
}

TransmissionLineClosed::TransmissionLineClosed(const TransmissionLineClosed& in)
{
	ranges = in.ranges;
}

std::complex<fvalue> TransmissionLineClosed::execute(fvalue omega)
{
	fvalue r = ranges[0].stepValue();
	fvalue q = ranges[1].stepValue();
	fvalue a = ranges[2].stepValue();
	fvalue l = ranges[3].stepValue();

	return std::sqrt(r/(q*std::pow(std::complex<fvalue>(0, omega), a)))*std::tanh(l*std::sqrt(std::pow(std::complex<fvalue>(0, omega), a)*r*q));
}

std::string TransmissionLineClosed::getCode(std::vector<std::string>& parameters)
{
	std::string r = getUniqueName() + "_0";
	std::string q = getUniqueName() + "_1";
	std::string a = getUniqueName() + "_2";
	std::string l = getUniqueName() + "_3";
	parameters.push_back(r);
	parameters.push_back(q);
	parameters.push_back(a);
	parameters.push_back(l);

	std::string first = "(std::sqrt("+r+"/("+q+"*std::pow(std::complex<fvalue>(0, omega), "+a+")))*";
	std::string second = "std::tanh("+l+"*std::sqrt(std::pow(std::complex<fvalue>(0, omega), "+a+")*"+r+"*"+q+")))";
	return first+second;
}

std::string TransmissionLineClosed::getTorchScript(std::vector<std::string>& parameters)
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

char TransmissionLineClosed::getComponantChar() const
{
	return TransmissionLineClosed::staticGetComponantChar();
}

size_t TransmissionLineClosed::paramCount() const
{
	return 4;
}

TransmissionLineClosed::~TransmissionLineClosed()
{
}
