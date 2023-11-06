#pragma once
#include <complex>
#include <vector>
#include <valarray>
#include <cassert>
#include <cmath>
#include <filesystem>

typedef float fvalue;

namespace eis
{

class DataPoint
{
public:
	std::complex<fvalue> im;
	fvalue omega;
	DataPoint() = default;
	DataPoint(std::complex<fvalue> imIn, fvalue omegaIn = 0): im(imIn), omega(omegaIn){}
	bool operator<(const DataPoint& in) const
	{
		return omega < in.omega;
	}
	bool operator>(const DataPoint& in) const
	{
		return omega > in.omega;
	}
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
	size_t count = 0;
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
	fvalue center() const
	{
		return (start+end)/2;
	}
	fvalue at(size_t index) const
	{
		assert(index < count || (index == 0 && count == 0));
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
	std::vector<fvalue> getRangeVector() const;

	static std::vector<Range> rangesFromParamString(const std::string& paramStr, size_t count);
};

class parse_errror: public std::exception
{
	std::string whatStr;
public:
	parse_errror(const std::string& whatIn): whatStr(whatIn)
	{}
	virtual const char* what() const noexcept override
	{
		return whatStr.c_str();
	}
};

class file_error: public std::exception
{
	std::string whatStr;
public:
	file_error(const std::string& whatIn): whatStr(whatIn)
	{}
	virtual const char* what() const noexcept override
	{
		return whatStr.c_str();
	}
};

class EisSpectra
{
public:
	static constexpr int F_VERSION_MAJOR = 1;
	static constexpr int F_VERSION_MINOR = 0;
	static constexpr int F_VERSION_PATCH = 0;
	static constexpr char F_MAGIC[] = "EISF";

public:
	std::vector<DataPoint> data;
	std::string model;
	std::string header;
	std::vector<double> labels;
	std::vector<std::string> labelNames;

public:
	EisSpectra(const std::vector<DataPoint>& dataIn, const std::string& modelIn, const std::string& headerIn,
			   std::vector<double> labelsIn = std::vector<double>(),
			   std::vector<std::string> labelNamesIn = std::vector<std::string>());
	EisSpectra(const std::vector<DataPoint>& dataIn, const std::string& modelIn, const std::string& headerIn,
			   std::vector<float> labelsIn, std::vector<std::string> labelNamesIn = std::vector<std::string>());
	EisSpectra(const std::vector<DataPoint>& dataIn, const std::string& modelIn, const std::string& headerIn,
			   std::vector<size_t> labelsIn, std::vector<std::string> labelNamesIn = std::vector<std::string>());
	EisSpectra(const std::vector<DataPoint>& dataIn, const std::string& modelIn, const std::string& headerIn,
			   size_t label, size_t maxLabel, std::vector<std::string> labelNamesIn = std::vector<std::string>());
	EisSpectra(const std::filesystem::path& path){*this = loadFromDisk(path);}
	EisSpectra(){}
	[[nodiscard]] static  EisSpectra loadFromDisk(const std::filesystem::path& path);
	[[nodiscard]] static EisSpectra loadFromStream(std::istream& path);
	void setLabel(size_t label, size_t maxLabel);
	size_t getLabel();
	void setSzLabels(std::vector<size_t> label);
	void setLabels(const std::vector<double>& labelsIn);
	void setLabels(const std::vector<float>& labelsIn);
	std::vector<size_t> getSzLabels() const;
	bool isMulticlass();
	std::vector<fvalue> getFvalueLabels();
	bool saveToDisk(const std::filesystem::path& path) const;
	void saveToStream(std::ostream& stream) const;
};

bool saveToDisk(const EisSpectra& data, const std::filesystem::path& path);

EisSpectra loadFromDisk(const std::filesystem::path& path);

std::pair<std::valarray<fvalue>, std::valarray<fvalue>> eisToValarrays(const std::vector<eis::DataPoint>& b);

fvalue eisDistance(const std::vector<eis::DataPoint>& a, const std::vector<eis::DataPoint>& b);

fvalue eisNyquistDistance(const std::vector<eis::DataPoint>& a, const std::vector<eis::DataPoint>& b);

}
