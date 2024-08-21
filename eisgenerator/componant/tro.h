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
#include "componant.h"

namespace eis
{

class TransmissionLineOpen: public Componant
{

public:
	TransmissionLineOpen(fvalue r, fvalue q, fvalue a, fvalue l);
	TransmissionLineOpen(std::string paramStr, size_t count = 10, bool defaultToRange = false);
	TransmissionLineOpen(const TransmissionLineOpen& in);
	virtual std::complex<fvalue> execute(fvalue omega) override;
	virtual size_t paramCount() const override;
	virtual std::vector<eis::Range> getDefaultParameters(bool range = true) const override;
	virtual ~TransmissionLineOpen();
	virtual char getComponantChar() const override;
	virtual std::string getCode(std::vector<std::string>& parameters) override;
	virtual std::string getTorchScript(std::vector<std::string>& parameters) override;
	virtual std::string componantName() const override {return "TransmissionLineOpen";}
	static constexpr char staticGetComponantChar(){return 'o';}
};

}
