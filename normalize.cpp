//SPDX-License-Identifier:         LGPL-3.0-or-later
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

#include "normalize.h"
#include <assert.h>
#include <cmath>
#include <complex>
#include <limits>
#include <kisstype/type.h>

#include "log.h"
#include "basicmath.h"

void eis::eraseSingularites(std::vector<eis::DataPoint>& data)
{
	for(size_t i = 0; i < data.size(); ++i)
	{
		if(std::isnan(data[i].im.real()) || std::isnan(data[i].im.imag()) || std::isinf(data[i].im.real()) || std::isinf(data[i].im.imag()) )
		{
			size_t left = i-1;
			size_t right = i+1;

			while(left > 1 && std::abs(eis::absGrad(data, left)) > 10)
				--left;
			while(right < data.size()-2 && std::abs(eis::absGrad(data, right)) > 10)
				++right;

			std::complex<fvalue> mean = (data[left].im + data[right].im)/static_cast<fvalue>(2.0);
			for(size_t j = left; j < right; ++j)
				data[j].im = mean;
		}
	}
}

void eis::normalize(std::vector<eis::DataPoint>& data)
{
	fvalue maxRe = std::numeric_limits<fvalue>::min();
	fvalue maxIm = std::numeric_limits<fvalue>::min();
	fvalue minRe = std::numeric_limits<fvalue>::max();
	for(const DataPoint& dataPoint : data)
	{
		maxRe = fabs(dataPoint.im.real()) > maxRe ? fabs(dataPoint.im.real()) : maxRe;
		maxIm = fabs(dataPoint.im.imag()) > maxIm ? fabs(dataPoint.im.imag()) : maxIm;

		if(minRe > dataPoint.im.real())
			minRe = dataPoint.im.real();
	}

	maxRe = maxRe == minRe ? 1 : maxRe-minRe;
	maxIm = maxIm == 0 ? 1 : maxIm;

	for(DataPoint& dataPoint : data)
	{
		dataPoint.im.real((dataPoint.im.real()-minRe) / maxRe);
		dataPoint.im.imag(dataPoint.im.imag() / maxIm);
	}
}

std::vector<eis::DataPoint> eis::reduceRegion(const std::vector<eis::DataPoint>& inData,
                                              fvalue gradThreshFactor, bool useSecondDeiv)
{
	if(inData.size() < 3)
		return inData;

	std::vector<eis::DataPoint> data = inData;
	//eis::eraseSingularites(data);
	eis::normalize(data);

	std::vector<fvalue> grads;
	grads.reserve(data.size());
	eis::Log(eis::Log::DEBUG)<<(useSecondDeiv ? "Second dirvative Grads:" : "First dirvative Grads:");
	for(size_t i = 0; i < data.size(); ++i)
	{
		grads.push_back(std::abs(eis::absGrad(data, i)));
		if(!useSecondDeiv)
			eis::Log(eis::Log::DEBUG)<<i<<": "<<inData[i].omega<<','<<grads.back();
	}

	if(useSecondDeiv)
	{
		std::vector<fvalue> omegas(data.size());
		for(size_t i = 0; i < data.size(); ++i)
			omegas[i] = data[i].omega;
		for(size_t i = 0; i < grads.size(); ++i)
		{
			grads[i] = (std::abs(eis::grad(grads, omegas, i)));
			eis::Log(eis::Log::DEBUG)<<i<<": "<<omegas[i]<<','<<grads[i];
		}
	}

	fvalue gradThresh;
	if(!useSecondDeiv)
		gradThresh = eis::median(grads)*gradThreshFactor;
	else
		gradThresh = 1e-12*gradThreshFactor;

	eis::Log(eis::Log::DEBUG)<<"Grad thresh is: "<<gradThresh;

	size_t start = 0;
	for(size_t i = 1; i < data.size()-1; ++i)
	{
		if(grads[i] < gradThresh)
			start = i;
		else
			break;
	}

	size_t end = data.size()-1;
	for(size_t i = data.size()-1; i > 1; --i)
	{
		if(grads[i] < gradThresh)
			end = i;
		else
			break;
	}

	eis::Log(eis::Log::DEBUG)<<"reduced range "<<start<<'-'<<end;

	data.erase(data.begin(), data.begin()+start);
	data.erase(data.begin()+end+1-start, data.end());

	return data;
}
