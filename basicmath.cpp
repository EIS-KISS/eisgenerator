//SPDX-License-Identifier:         LGPL-3.0-or-later
//
// eisgenerator - a shared library and application to generate EIS spectra
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

#include "basicmath.h"
#include <algorithm>
#include <random>
#include <limits>
#include <stdexcept>

#include "eistype.h"
#include "log.h"
#include "linearregession.h"

static size_t gradIndex(size_t dataSize, size_t inputIndex)
{
	if(inputIndex == 0)
		inputIndex = 1;
	else if(inputIndex > dataSize-2)
		inputIndex = dataSize-2;
	return inputIndex;
}

std::complex<fvalue> eis::absGrad(const std::vector<eis::DataPoint>& data, size_t index)
{
	if(data.size() < 3)
		return std::complex<fvalue>(1,1);

	index = gradIndex(data.size(), index);

	return std::complex<fvalue>(std::abs((data[index+1].im.real()-data[index-1].im.real())/(data[index+1].omega-data[index-1].omega)),
								std::abs((data[index+1].im.imag()-data[index-1].im.imag())/(data[index+1].omega-data[index-1].omega)));
}

fvalue eis::grad(const std::vector<fvalue>& data, const std::vector<fvalue>& omega, size_t index)
{
	assert(data.size() == omega.size());
	if(data.size() < 3)
		return 0;

	index = gradIndex(data.size(), index);

	return (data[index+1]-data[index-1])/(omega[index+1]-omega[index-1]);
}

std::complex<fvalue> eis::grad(const std::vector<eis::DataPoint>& data, size_t index)
{
	if(data.size() < 3)
		return std::complex<fvalue>(1,1);

	index = gradIndex(data.size(), index);

	return std::complex<fvalue>((data[index+1].im.real()-data[index-1].im.real())/(data[index+1].omega-data[index-1].omega),
								(data[index+1].im.imag()-data[index-1].im.imag())/(data[index+1].omega-data[index-1].omega));
}

std::complex<fvalue> eis::mean(const std::vector<eis::DataPoint>& data)
{
	fvalue accumRe = 0;
	fvalue accumIm = 0;

	for(const eis::DataPoint& point : data)
	{
		accumRe += point.im.real();
		accumIm += point.im.imag();
	}

	accumRe /= data.size();
	accumIm /= data.size();
	return std::complex<fvalue>(accumRe, accumIm);
}

static inline fvalue medianTrampoline(const std::vector<fvalue> data)
{
	return eis::median(data);
}

std::complex<fvalue> eis::median(const std::vector<eis::DataPoint>& data)
{
	if(data.empty())
		return std::complex<fvalue>(0,0);
	else if(data.size() == 1)
		return data[0].im;

	std::vector<fvalue> imagParts;
	imagParts.reserve(data.size());
	std::vector<fvalue> realParts;
	realParts.reserve(data.size());

	for(const eis::DataPoint& point : data)
	{
		imagParts.push_back(point.im.imag());
		realParts.push_back(point.im.real());
	}

	fvalue realMean = medianTrampoline(realParts);
	fvalue imagMean = medianTrampoline(imagParts);

	return std::complex<fvalue>(realMean, imagMean);
}

fvalue eis::mean(const std::vector<fvalue>& data)
{
	fvalue accum = 0;

	for(fvalue point : data)
		accum += point;

	return accum/data.size();
}

fvalue eis::median(std::vector<fvalue> data)
{
	if(data.empty())
		return 0;
	else if(data.size() == 1)
		return data[0];

	std::sort(data.begin(), data.end());

	if(data.size() % 2 == 0)
		return (data[data.size()/2] + data[data.size()/2-1])/2;
	else
		return data[data.size()/2];
}

std::vector<eis::DataPoint> eis::rescale(const std::vector<eis::DataPoint>& data, size_t outputSize)
{
	std::vector<eis::DataPoint> output(outputSize);
	for(size_t i = 0; i < output.size(); ++i)
	{
		fvalue position = static_cast<double>(i) / (output.size()-1);
		fvalue sourcePosF = (data.size()-1)*position;
		size_t sourcePos = (data.size()-1)*position;
		fvalue frac = sourcePosF - sourcePos;
		output[i].im = data[sourcePos].im*(1-frac) + data[sourcePos+1].im*frac;
		output[i].omega = data[sourcePos].omega*(1-frac) + data[sourcePos+1].omega*frac;
	}
	return output;
}

void eis::noise(std::vector<eis::DataPoint>& data, double amplitude, bool relative)
{
	std::random_device rd;
	std::default_random_engine rande(rd());

	for(eis::DataPoint& point : data)
	{
		double realNoise = (static_cast<double>(rande() >> 1) / (rande.max() >> 1))*amplitude;
		point.im.real(relative ? point.im.real()+realNoise/point.im.real() : point.im.real()+realNoise);

		double imgNoise = (static_cast<double>(rande() >> 1) / (rande.max() >> 1))*amplitude;
		point.im.imag(relative ? point.im.imag()+imgNoise/point.im.imag() : point.im.imag()+imgNoise);
	}
}

fvalue eis::pearsonCorrelation(const std::vector<eis::DataPoint>& data)
{
	std::complex<fvalue> meanValue = mean(data);

	fvalue sumDeltaReDeltaIm = 0;
	fvalue sumDeltaReSq = 0;
	fvalue sumDeltaImSq = 0;

	for(const eis::DataPoint& point : data)
	{
		sumDeltaReDeltaIm += (point.im.real()-meanValue.real())*(point.im.imag()-meanValue.imag());
		sumDeltaReSq += pow(point.im.real()-meanValue.real(), 2);
		sumDeltaImSq += pow(point.im.imag()-meanValue.imag(), 2);
	}

	return sumDeltaReDeltaIm/(sqrt(sumDeltaReSq)*sqrt(sumDeltaImSq));
}

fvalue eis::nyquistAreaVariance(const std::vector<eis::DataPoint>& data, eis::DataPoint* centroid)
{
	assert(data.size() > 2);

	eis::DataPoint localCentroid(std::complex<fvalue>(0,0));
	if(!centroid)
	{
		centroid = &localCentroid;
		for(const eis::DataPoint& point : data)
			*centroid = *centroid + point;
		*centroid = (*centroid)/data.size();
	}

	double realVar = 0;
	double imagVar = 0;
	double realImagVar = 0;
	for(const eis::DataPoint& point : data)
	{
		eis::DataPoint a = point-*centroid;
		realVar += std::pow(a.im.real(), 2);
		imagVar += std::pow(a.im.imag(), 2);
		realImagVar += a.im.real()*a.im.imag();
	}
	realVar /= data.size();
	imagVar /= data.size();
	realImagVar /= data.size();
	return std::sqrt(realVar+imagVar+std::pow(realImagVar, 2));
}

fvalue eis::maximumNyquistJump(const std::vector<eis::DataPoint>& data)
{
	assert(data.size() > 1);
	fvalue maxDist = std::numeric_limits<fvalue>::min();
	for(size_t i = 1; i < data.size(); ++i)
	{
		eis::DataPoint a = data[i]-data[i-1];
		fvalue realVar = std::pow(a.im.real(), 2);
		fvalue imagVar = std::pow(a.im.imag(), 2);
		fvalue dist = std::sqrt(realVar+imagVar);
		maxDist = maxDist < dist ? dist : maxDist;
	}
	return maxDist;
}

void eis::removeDuplicates(std::vector<eis::DataPoint>& data)
{
	std::sort(data.begin(), data.end());

	std::vector<eis::DataPoint>::iterator it = data.begin();
	while((it = std::adjacent_find(data.begin(), data.end(),
		[](const eis::DataPoint& a, const eis::DataPoint& b){return fvalueEq(a.omega, b.omega);})) != data.end())
	{
		data.erase(it);
	}
}

bool eis::fvalueEq(fvalue a, fvalue b, unsigned int ulp)
{
	fvalue epsilon = std::numeric_limits<fvalue>::epsilon()*std::fabs(a+b)*ulp;
	return a - epsilon <= b && a + epsilon >= b;
}

static std::pair<std::vector<eis::DataPoint>::const_iterator, std::vector<eis::DataPoint>::const_iterator>
getLrClosest(const eis::DataPoint& dp, std::vector<eis::DataPoint>::const_iterator start, std::vector<eis::DataPoint>::const_iterator end)
{

	std::vector<eis::DataPoint>::const_iterator left = end;
	fvalue distLeft = std::numeric_limits<fvalue>::max();
	std::vector<eis::DataPoint>::const_iterator right = end;
	fvalue distRight = std::numeric_limits<fvalue>::max();

	for(std::vector<eis::DataPoint>::const_iterator it = start; it != end; it++)
	{
		if(eis::fvalueEq(it->omega, dp.omega))
			return {it, it};
		fvalue dist = it->omega-dp.omega;
		bool sign = std::signbit(dist);
		dist = std::abs(dist);

		if(sign && (left == end || dist < distLeft))
		{
			distLeft = dist;
			left = it;
		}
		else if(!sign && (right == end || dist < distRight))
		{
			distRight = dist;
			right = it;
		}
	}
	return {left, right};
}

static bool omegaCompeare(std::pair<fvalue, std::vector<eis::DataPoint>::const_iterator> a,
                          std::pair<fvalue, std::vector<eis::DataPoint>::const_iterator> b)
{
	return a.first < b.first;
}

static std::vector<std::pair<fvalue, std::vector<eis::DataPoint>::const_iterator>>
getSortedOmegaDistances(fvalue omega,
                        std::vector<eis::DataPoint>::const_iterator start,
                        std::vector<eis::DataPoint>::const_iterator end)
{
	std::vector<std::pair<fvalue, std::vector<eis::DataPoint>::const_iterator>> out;

	for(std::vector<eis::DataPoint>::const_iterator it = start; it != end; it++)
		out.push_back({std::abs(it->omega-omega), it});
	std::sort(out.begin(), out.end(), omegaCompeare);
	return out;
}

static eis::DataPoint linearInterpolatePoint(fvalue omega, const eis::DataPoint& left, const eis::DataPoint& right)
{
	assert(left.omega <= omega);
	assert(right.omega >= omega);

	fvalue omegaDif = right.omega - left.omega;
	std::complex<fvalue> sloap = (right.im - left.im)/omegaDif;
	return eis::DataPoint(left.im+sloap*(omega - left.omega), omega);
}

static eis::DataPoint linearExtrapoloatePoint(fvalue omega, const std::vector<eis::DataPoint>& data)
{
	if(data.size() < 3)
		throw std::invalid_argument("extrapolation requires at least 3 points");

	std::vector<std::pair<fvalue, std::vector<eis::DataPoint>::const_iterator>> dist;
	dist = getSortedOmegaDistances(omega, data.begin(), data.end());

	std::vector<fvalue> omegas;
	std::vector<fvalue> im;
	std::vector<fvalue> re;
	omegas.reserve(dist.size());
	im.reserve(dist.size());
	re.reserve(dist.size());
	for(size_t i = 0; i < std::min(static_cast<size_t>(6), data.size()); ++i)
	{
		omegas.push_back(log10(dist[i].second->omega));
		im.push_back(dist[i].second->im.imag());
		re.push_back(dist[i].second->im.real());
	}

	LinearRegessionCalculator realReg(omegas, re);
	LinearRegessionCalculator imagReg(omegas, im);

	eis::Log(eis::Log::DEBUG)<<"Real regression for "<<omega<<":\n\toffset: "<<realReg.offset
		<<"\n\tsloap: "<<realReg.slope<<"\n\tstderror: "<<realReg.stdError;
	eis::Log(eis::Log::DEBUG)<<"Imag regression for "<<omega<<":\n\toffset: "<<imagReg.offset
		<<"\n\tsloap: "<<imagReg.slope<<"\n\tstderror: "<<imagReg.stdError;

	if(realReg.stdError > 3 || imagReg.stdError > 3)
		throw std::invalid_argument("input data must be sufficiently linear");

	std::complex<fvalue> expIm(realReg.slope*log10(omega)+realReg.offset, imagReg.slope*log10(omega)+imagReg.offset);

	return eis::DataPoint(expIm, omega);
}

std::vector<eis::DataPoint> eis::fitToFrequencies(std::vector<fvalue> omegas, const std::vector<eis::DataPoint>& data, bool linearExtrapolation)
{
	std::vector<eis::DataPoint> out;
	out.reserve(omegas.size());
	for(fvalue omega : omegas)
		out.push_back(eis::DataPoint({0,0}, omega));

	eis::Log(eis::Log::DEBUG)<<__func__<<':';

	for(eis::DataPoint& dp : out)
	{
		auto lr = getLrClosest(dp, data.begin(), data.end());

		eis::Log(eis::Log::DEBUG)<<"\tValue for "<<dp.omega;
		if(lr.first != data.end())
			eis::Log(eis::Log::DEBUG)<<"\tLeft "<<lr.first->omega<<','<<lr.first->im;
		if(lr.second != data.end())
			eis::Log(eis::Log::DEBUG)<<"\tRight "<<lr.second->omega<<','<<lr.second->im;

		if(lr.first == lr.second)
		{
			dp.im = lr.first->im;
		}
		else if(lr.first != data.end() && lr.second != data.end())
		{
			dp = linearInterpolatePoint(dp.omega, *lr.first, *lr.second);
		}
		else if(lr.first != data.end() && lr.second == data.end())
		{
			try
			{
				if(!linearExtrapolation)
					dp.im = lr.first->im;
				else
					dp = linearExtrapoloatePoint(dp.omega, data);
			}
			catch (const std::invalid_argument& ex)
			{
				dp.im = lr.first->im;
			}
		}
		else if(lr.first == data.end() && lr.second != data.end())
		{
			try
			{
				if(!linearExtrapolation)
					dp.im = lr.second->im;
				else
					dp = linearExtrapoloatePoint(dp.omega, data);
			}
			catch (const std::invalid_argument& ex)
			{
				dp.im = lr.second->im;
			}
		}
		else
		{
			assert(false);
		}
	}

	return out;
}

void eis::difference(std::vector<eis::DataPoint>& a, const std::vector<eis::DataPoint>& b)
{
	assert(a.size() == b.size());
	for(size_t i = 0; i < a.size(); ++i)
		a[i] = a[i] - b[i];
}
