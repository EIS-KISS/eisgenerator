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

#include "linearregession.h"
#include <stdexcept>
#include <cmath>

LinearRegessionCalculator::LinearRegessionCalculator(const std::vector < fvalue > &xValues, const std::vector < fvalue > &yValues)
{
	if(xValues.size() == yValues.size())
	{
		this->xValues = xValues;
		this->yValues = yValues;

		fvalue sumY = 0;
		fvalue sumX = 0;
		fvalue sumXTimesY = 0;
		fvalue sumSquaredX = 0;
		fvalue sumSquaredY = 0;
		for(unsigned i = 0; i < xValues.size(); i++)
		{
			sumY += yValues[i];
			sumX += xValues[i];
			sumSquaredX += xValues[i] * xValues[i];
			sumSquaredY += yValues[i] * yValues[i];
			sumXTimesY += xValues[i] * yValues[i];
		}

		slope = (sumXTimesY - (sumX * sumY) / xValues.size()) / (sumSquaredX - (sumX * sumX) / xValues.size());
		offset = sumY / xValues.size() - slope * (sumX / xValues.size());

		fvalue error =
		    (xValues.size() * sumSquaredY - sumY * sumY -
		     slope * slope * (xValues.size() * sumSquaredX - sumX * sumX)) / (xValues.size() * (xValues.size() - 2));
		stdError = sqrt((error * error * xValues.size()) / (xValues.size() * sumSquaredX - sumX * sumX));
	}
	else
	{
		throw std::invalid_argument("xValues and yValues need to be the same size");
	}
}
