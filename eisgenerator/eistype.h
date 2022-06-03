#pragma once
#include <complex>
#include <vector>

typedef double fvalue;

namespace eis
{
struct DataPoint
{
	std::complex<fvalue> im;
	fvalue omega;
};

struct Range
{
	fvalue start;
	fvalue end;
	size_t count;
	bool log = false;
	char type = 'x';

	fvalue stepSize() const
	{
		return (end-start)/count;
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
	Range(fvalue startI, fvalue endI, size_t countI, bool logI = false, char typeI = 'x'): start(startI), end(endI), count(countI), log(logI), type(typeI){}
	Range() = default;
};

bool saveToDisk(const std::vector<DataPoint>& data, std::string fileName);

}
