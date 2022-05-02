#include "basicmath.h"
#include <algorithm>

#include "eistype.h"

std::complex<fvalue> eis::absGrad(const std::vector<eis::DataPoint>& data, size_t index)
{
	if(data.size() < 3)
		return std::complex<fvalue>(1,1);

	if(index == 0)
		index = 1;
	else if(index > data.size()-2)
		index = data.size()-2;

	return std::complex<fvalue>(std::abs((data[index+1].im.real()-data[index-1].im.real())/(data[index+1].omega-data[index-1].omega)),
								std::abs((data[index+1].im.imag()-data[index-1].im.imag())/(data[index+1].omega-data[index-1].omega)));
}

std::complex<fvalue> eis::grad(const std::vector<eis::DataPoint>& data, size_t index)
{
	if(data.size() < 3)
		return std::complex<fvalue>(1,1);

	if(index == 0)
		index = 1;
	else if(index > data.size()-2)
		index = data.size()-2;

	return std::complex<fvalue>((data[index+1].im.real()-data[index-1].im.real())/(data[index+1].omega-data[index-1].omega),
								(data[index+1].im.imag()-data[index-1].im.imag())/(data[index+1].omega-data[index-1].omega));
}

std::complex<fvalue> mean(const std::vector<eis::DataPoint>& data)
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

std::complex<fvalue> median(const std::vector<eis::DataPoint>& data)
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

	std::sort(imagParts.begin(), imagParts.end());
	std::sort(realParts.begin(), realParts.end());


	if(data.size() % 2 == 0)
	{
		fvalue real = (realParts[data.size()/2] + realParts[data.size()/2-1])/2;
		fvalue imag = (imagParts[data.size()/2] + imagParts[data.size()/2-1])/2;
		return std::complex<fvalue>(real, imag);
	}
	else
	{
		size_t index = data.size()/2;
		return std::complex<fvalue>(realParts[index], imagParts[index]);
	}

}
