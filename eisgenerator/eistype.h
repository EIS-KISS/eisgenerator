#pragma once
#include <complex>
#include <vector>
#include <cassert>
#include <cmath>

typedef double fvalue;

namespace eis
{
class DataPoint
{
public:
	std::complex<fvalue> im;
	fvalue omega;
	DataPoint() = default;
	DataPoint(std::complex<fvalue> imIn, fvalue omegaIn = 0): im(imIn), omega(omegaIn){}
	bool operator==(const DataPoint& in) const
	{
		return im == in.im;
	}
	DataPoint operator-(const DataPoint& in) const
	{
		DataPoint out(*this);
		out.im = out.im - in.im;
		return out;
	}
	DataPoint operator+(const DataPoint& in) const
	{
		DataPoint out(*this);
		out.im = out.im + in.im;
		return out;
	}
	DataPoint operator/(fvalue in) const
	{
		DataPoint out(*this);
		out.im = out.im / in;
		return out;
	}
	DataPoint operator*(fvalue in) const
	{
		DataPoint out(*this);
		out.im = out.im * in;
		return out;
	}
	DataPoint operator/=(fvalue in)
	{
		im = im / in;
		return *this;
	}
	fvalue complexVectorLength() const
	{
		return std::sqrt(std::pow(im.real(), 2) + std::pow(im.imag(), 2));
	}
};

class Range
{
public:
	fvalue start;
	fvalue end;
	size_t count;
	size_t step = 0;
	bool log = false;

	fvalue stepSize() const
	{
		fvalue startL = log ? log10(start) : start;
		fvalue endL = log ? log10(end) : end;
		return (endL-startL)/(count-1);
	}
	fvalue stepValue() const
	{
		return at(step);
	}
	fvalue at(size_t index) const
	{
		assert(index < count);
		if(count < 2)
			return start;
		return log ? pow(10, stepSize()*index+log10(start)) : stepSize()*index+start;
	}
	fvalue operator[](size_t index) const
	{
		return at(index);
	}
	Range operator*(fvalue in) const
	{
		return Range(start*in, end*in, count, log);
	}
	Range operator/(fvalue in) const
	{
		return operator*(static_cast<fvalue>(1.0)/in);
	}
	Range operator*(int in) const
	{
		return operator*(static_cast<fvalue>(in));
	}
	Range operator/(int in) const
	{
		return operator*(static_cast<fvalue>(1.0)/in);
	}
	Range(fvalue startI, fvalue endI, size_t countI, bool logI = false): start(startI), end(endI), count(countI), log(logI){}
	Range() = default;
	void print(int level) const;
	std::string getString() const;
	bool isSane() const;

	static std::vector<Range> rangesFromParamString(const std::string& paramStr, size_t count);
};

bool saveToDisk(const std::vector<DataPoint>& data, const std::string& fileName, std::string headStr = "");

fvalue eisDistance(const std::vector<eis::DataPoint>& a, const std::vector<eis::DataPoint>& b);

fvalue eisNyquistDistance(const std::vector<eis::DataPoint>& a, const std::vector<eis::DataPoint>& b);

}
