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
#include <complex>
#include <iostream>
#include <vector>
#include <string>
#include <kisstype/type.h>

namespace eis
{

class Componant
{
	protected:
		std::vector<eis::Range> ranges;
		std::string uniqueName;

	public:
		virtual std::complex<fvalue> execute(fvalue omega)
		{
			(void)omega;
			std::cout<<"warning incompleat model\n";
			return std::complex<fvalue> (1,0);
		}

		virtual void setParamRanges(const std::vector<eis::Range>& ranges);
		virtual std::vector<eis::Range>& getParamRanges();
		virtual std::vector<eis::Range> getParamRanges() const;
		virtual std::vector<eis::Range> getDefaultParameters(bool range = true) const;
		virtual size_t paramCount() const {return 0;};
		virtual ~Componant() = default;
		virtual char getComponantChar() const = 0;
		virtual std::string getComponantString(bool currentValue = true) const;
		virtual std::string componantName() const = 0;
		virtual std::string getCode(std::vector<std::string>& parameters);
		virtual std::string getTorchScript(std::vector<std::string>& parameters);
		virtual bool compileable();

		std::string getUniqueName();

		static Componant* copy(Componant* componant);
		static Componant* createNewComponant(char componant, std::string paramStr = "", size_t count = 1, bool defaultToRange = false);
		static bool isValidComponantChar(char componant);
};

}
