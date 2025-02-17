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

#include "componant/paralellseriel.h"
#include "componant/componant.h"
#include "type.h"

using namespace eis;

std::vector<bool> ParallelSerial::contributes(fvalue omega, fvalue threshold)
{
	std::vector<fvalue> ratios = contributionRatio(omega);
	std::vector<bool> out(ratios.size());
	for(size_t i = 0; i < ratios.size(); ++i)
		out[i] = ratios[i] > threshold;
	return out;
}

Parallel::Parallel(std::vector<Componant*> componantsIn): componants(componantsIn)
{
}

Parallel::Parallel(const Parallel& in)
{
	operator=(in);
}

void Parallel::operator=(const Parallel& in)
{
	componants.clear();
	componants.reserve(in.componants.size());
	for(Componant* componant : in.componants)
		componants.push_back(copy(componant));
}

Parallel::~Parallel()
{
	for(Componant* componant : componants)
		delete componant;
}

std::complex<fvalue> Parallel::execute(fvalue omega)
{
	std::complex<fvalue> accum(0,0);
	for(Componant* componant : componants)
	{
		accum += std::complex<fvalue>(1,0)/componant->execute(omega);
	}
	return std::complex<fvalue>(1,0)/accum;
}

char Parallel::getComponantChar() const
{
	return staticGetComponantChar();
}

std::string Parallel::getComponantString(bool currentValue) const
{
	std::string out("(");
	for(Componant* componant : componants)
		out.append(componant->getComponantString(currentValue));
	out.push_back(')');
	return out;
}

bool Parallel::compileable()
{
	for(Componant* componant : componants)
	{
		if(!componant->compileable())
			return false;
	}
	return true;
}

std::string Parallel::getCode(std::vector<std::string>& parameters)
{
	std::string out = "std::complex<fvalue>(1,0)/(";
	for(Componant* componant : componants)
	{
		out += "std::complex<fvalue>(1,0)/(" + componant->getCode(parameters) + ") + ";
	}
	out.pop_back();
	out.pop_back();
	out.pop_back();
	out.push_back(')');
	return out;
}

std::string Parallel::getTorchScript(std::vector<std::string>& parameters)
{
	std::string out = "1/(";
	for(Componant* componant : componants)
	{
		out += "1/(" + componant->getTorchScript(parameters) + ") + ";
	}
	out.pop_back();
	out.pop_back();
	out.pop_back();
	out.push_back(')');
	return out;
}

std::vector<fvalue> Parallel::contributionRatio(fvalue omega)
{
	std::vector<fvalue> out(componants.size());
	for(size_t i = 0; i < componants.size(); ++i)
		out[i] = std::abs(componants[i]->execute(omega));
	fvalue max = *std::min_element(out.begin(), out.end());
	for(fvalue& val : out)
		val = max / val;
	return out;
}

Serial::Serial(std::vector<Componant*> componantsIn): componants(componantsIn)
{
}

Serial::Serial(const Serial& in)
{
	operator=(in);
}

void Serial::operator=(const Serial& in)
{
	componants.clear();
	componants.reserve(in.componants.size());
	for(Componant* componant : in.componants)
		componants.push_back(copy(componant));
}

Serial::~Serial()
{
	for(Componant* componant : componants)
		delete componant;
}

std::complex<fvalue> Serial::execute(fvalue omega)
{
	std::complex<fvalue> accum(0,0);
	for(Componant* componant : componants)
	{
		accum += componant->execute(omega);
	}
	return accum;
}

char Serial::getComponantChar() const
{
	return staticGetComponantChar();
}

std::string Serial::getComponantString(bool currentValue) const
{
	std::string out("(");
	for(Componant* componant : componants)
	{
		out.append(componant->getComponantString(currentValue));
		out.push_back('-');
	}
	out.back() = ')';
	return out;
}

bool Serial::compileable()
{
	for(Componant* componant : componants)
	{
		if(!componant->compileable())
			return false;
	}
	return true;
}

std::string Serial::getCode(std::vector<std::string>& parameters)
{
	std::string out = "(";
	for(Componant* componant : componants)
	{
		out += "(" + componant->getCode(parameters) + ") + ";
	}
	out.pop_back();
	out.pop_back();
	out.pop_back();
	out.push_back(')');
	return out;
}

std::string Serial::getTorchScript(std::vector<std::string>& parameters)
{
	std::string out = "(";
	for(Componant* componant : componants)
	{
		out += "(" + componant->getTorchScript(parameters) + ") + ";
	}
	out.pop_back();
	out.pop_back();
	out.pop_back();
	out.push_back(')');
	return out;
}

std::vector<fvalue> Serial::contributionRatio(fvalue omega)
{
	std::vector<fvalue> out(componants.size());
	for(size_t i = 0; i < componants.size(); ++i)
		out[i] = std::abs(componants[i]->execute(omega));
	fvalue max = *std::max_element(out.begin(), out.end());
	for(fvalue& val : out)
		val = val / max;
	return out;
}
