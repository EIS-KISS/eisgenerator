#include "basicmath.h"
#include <algorithm>
#include <random>

#include "eistype.h"

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
		double position = static_cast<double>(i) / (output.size()-1);
		double sourcePosF = (data.size()-1)*position;
		size_t sourcePos = (data.size()-1)*position;
		double frac = sourcePosF - sourcePos;
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

bool eis::fvalueEq(fvalue a, fvalue b, fvalue epsilon)
{
	return a - epsilon < b && a + epsilon > b;
}
