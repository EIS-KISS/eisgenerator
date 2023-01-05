#pragma once
#include <complex>
#include <vector>
#include <cassert>

typedef double fvalue;

namespace eis
{
struct DataPoint
{
	std::complex<fvalue> im;
	fvalue omega;
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
		return log ? pow(10, stepSize()*index+start) : stepSize()*index+start;
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

	static std::vector<Range> rangesFromParamString(const std::string& paramStr, size_t count);
};

bool saveToDisk(const std::vector<DataPoint>& data, std::string fileName);

}
