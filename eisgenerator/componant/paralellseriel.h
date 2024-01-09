//SPDX-License-Identifier:         LGPL-3.0-or-later
/* * eisgenerator - a shared libary and application to generate EIS spectra
 * Copyright (C) 2022-2024 Carl Philipp Klemm <carl@uvos.xyz>
 *
 * This file is part of eisgenerator.
 *
 * eisgenerator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eisgenerator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with eisgenerator.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <vector>
#include <complex>
#include "componant.h"

namespace eis
{

class Parallel: public Componant
{
public:
	std::vector<Componant*> componants;

	Parallel(std::vector<Componant*> componantsIn = std::vector<Componant*>());
	Parallel(const Parallel& in);
	void operator=(const Parallel& in);
	~Parallel();
	virtual std::complex<fvalue> execute(fvalue omaga) override;
	virtual char getComponantChar() const override;
	virtual std::string getComponantString(bool currentValue = true) const override;
	static constexpr char staticGetComponantChar(){return 'd';}
	virtual std::string componantName() const override {return "Parallel";}
	virtual bool compileable() override;
	virtual std::string getCode(std::vector<std::string>& parameters) override;
	virtual std::string getTorchScript(std::vector<std::string>& parameters) override;
};

class Serial: public Componant
{
public:
	std::vector<Componant*> componants;

	Serial(std::vector<Componant*> componantsIn = std::vector<Componant*>());
	Serial(const Serial& in);
	void operator=(const Serial& in);
	~Serial();
	virtual std::complex<fvalue> execute(fvalue omaga) override;
	virtual char getComponantChar() const override;
	virtual std::string getComponantString(bool currentValue = true) const override;
	static constexpr char staticGetComponantChar(){return 's';}
	virtual std::string componantName() const override {return "Serial";}
	virtual bool compileable() override;
	virtual std::string getCode(std::vector<std::string>& parameters) override;
	virtual std::string getTorchScript(std::vector<std::string>& parameters) override;
};

}
