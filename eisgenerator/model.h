#pragma once

#include <complex>
#include <string>
#include <vector>
#include <functional>
#include "eistype.h"
#include "componant.h"

namespace eis
{

struct DataPoint
{
	std::complex<double> im;
	double omega;
};

struct Range
{
	double start;
	double end;
	size_t count;
	bool log = false;

	double stepSize() const
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
	Range(double startI, double endI, size_t countI, bool logI = false): start(startI), end(endI), count(countI), log(logI){}
	Range() = default;
};

class Model
{
private:
	size_t opposingBraket(const std::string& str, size_t index, char bracketChar = ')');
	size_t deepestBraket(const std::string& str);
	Componant *processBrackets(std::string& str, size_t& bracketCounter);
	Componant *processBracket(std::string& str);
	std::string getParamStr(const std::string& str, size_t index);
	void addComponantToFlat(Componant* componant);

private:
	Componant *_model = nullptr;
	std::vector<Componant*> _bracketComponants;
	std::string _modelStr;
	std::vector<Componant*> _flatComponants;

public:
	Model(const std::string& str);
	Model(const Model& in);
	Model& operator=(const Model& in);
	~Model();
	DataPoint execute(double omaga);
	std::vector<DataPoint> executeSweep(const Range& omega);
	bool executeParamSweep(const std::vector<Range>& componantRanges, const Range& omega, std::function<void(std::vector<DataPoint>&, const std::vector<double>&)> dataCb);
	std::vector<DataPoint> executeParamByIndex(const std::vector<Range>& componantRanges, const Range& omega, size_t index);

	std::string getModelStr();
	std::vector<Componant*> getFlatComponants();
	std::vector<double> getFlatParameters();
	size_t getFlatParametersCount();
	bool setFlatParameters(const std::vector<double>& parameters);

	static std::vector<double> getSweepParamByIndex(const std::vector<Range>& componantRanges, size_t index);
	static size_t getRequiredStepsForSweeps(const std::vector<Range>& componantRanges);
};

}
