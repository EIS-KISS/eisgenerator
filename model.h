#pragma once

#include <complex>
#include <string>
#include <vector>
#include <functional>

#include "componant.h"

class Model
{
public:
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

		Range(double startI, double endI, size_t countI): start(startI), end(endI), count(countI){}
		Range() = default;
	};

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
	std::vector<DataPoint> sweep(const Range& omega);
	bool sweepParams(const std::vector<Range>& componantRanges, const Range& omega, std::function<void(std::vector<DataPoint>&, const std::vector<double>&)> dataCb);
	std::string getModelStr();
	std::vector<Componant*> getFlatComponants();
	size_t getFlatParametersCount();
	bool setFlatParameters(const std::vector<double>& parameters);
};
