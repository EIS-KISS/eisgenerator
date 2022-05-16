#include "normalize.h"
#include <assert.h>
#include <cmath>
#include <complex>
#include <limits>
#include "eistype.h"
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
	fvalue maxRe = 0;
	fvalue maxIm = 0;
	for(const DataPoint& dataPoint : data)
	{
		maxRe = fabs(dataPoint.im.real()) > maxRe ? fabs(dataPoint.im.real()) :  maxRe;
		maxIm = fabs(dataPoint.im.imag()) > maxIm ? fabs(dataPoint.im.imag()) : maxIm;
	}

	maxRe = maxRe == 0 ? 1 : maxRe;
	maxIm = maxIm == 0 ? 1 : maxIm;

	for(DataPoint& dataPoint : data)
	{
		dataPoint.im.real(dataPoint.im.real() / maxRe);
		dataPoint.im.imag(dataPoint.im.imag() / maxIm);
	}
}

std::vector<eis::DataPoint> eis::reduceRegion(const std::vector<eis::DataPoint>& inData, fvalue gradThreshFactor)
{
	if(inData.size() < 3)
		return inData;

	std::vector<eis::DataPoint> data = inData;
	//eis::eraseSingularites(data);
	eis::normalize(data);

	std::vector<fvalue> grads;
	grads.reserve(data.size());
	eis::Log(eis::Log::DEBUG)<<"Grads:";
	for(size_t i = 0; i < data.size(); ++i)
	{
		grads.push_back(std::abs(eis::absGrad(data, i)));
		eis::Log(eis::Log::DEBUG)<<i<<": "<<inData[i].omega<<','<<grads.back();
	}

	fvalue gradThresh = eis::median(grads)*gradThreshFactor;

	eis::Log(eis::Log::DEBUG)<<"Grad thresh is:"<<','<<gradThresh;

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
	data.erase(data.begin()+end, data.end());

	return data;
}
