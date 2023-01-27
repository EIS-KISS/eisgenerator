#pragma once

#include <complex>
#include <cstddef>
#include <string>
#include <vector>
#include <functional>
#include "eistype.h"
#include "componant.h"

namespace eis
{

class Model
{
private:
	Componant *processBrackets(std::string& str, size_t& bracketCounter, size_t paramSweepCount, bool defaultToRange);
	Componant *processBracket(std::string& str, size_t paramSweepCount, bool defaultToRange);
	std::string getParamStr(const std::string& str, size_t index);
	static size_t paramSkipIndex(const std::string& str, size_t index);
	static void addComponantToFlat(Componant* componant, std::vector<Componant*>* flatComponants);

	static void sweepThreadFn(std::vector<std::vector<DataPoint>>* data, Model* model, size_t start, size_t stop, const Range& omega);

	size_t getActiveParameterCount();

private:
	Componant *_model = nullptr;
	std::vector<Componant*> _bracketComponants;
	std::string _modelStr;
	std::vector<Componant*> _flatComponants;

public:
	Model(const std::string& str, size_t paramSweepCount = 100, bool defaultToRange = true);
	Model(const Model& in);
	Model& operator=(const Model& in);
	~Model();
	size_t setParamSweepCountClosestTotal(size_t totalCount);
	DataPoint execute(fvalue omaga, size_t index = 0);
	std::vector<DataPoint> executeSweep(const Range& omega, size_t index = 0);
	std::vector<std::vector<DataPoint>> executeAllSweeps(const Range& omega);
	std::string getModelStr() const;
	std::string getModelStrWithParam(size_t index);
	std::string getModelStrWithParam() const;
	std::vector<Componant*> getFlatComponants(Componant *model = nullptr);
	size_t getParameterCount();
	bool isReady();
	void resolveSteps(int64_t index);
	size_t getRequiredStepsForSweeps();
	bool isParamSweep();
};

}
