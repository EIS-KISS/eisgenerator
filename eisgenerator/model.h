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
	size_t opposingBraket(const std::string& str, size_t index, char bracketChar = ')');
	size_t deepestBraket(const std::string& str);
	Componant *processBrackets(std::string& str, size_t& bracketCounter);
	Componant *processBracket(std::string& str);
	std::string getParamStr(const std::string& str, size_t index);
	static void addComponantToFlat(Componant* componant, std::vector<Componant*>* flatComponants);
	Componant* flatParameterToFlatComponant(size_t parameterIndex);

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
	DataPoint execute(fvalue omaga, Componant* model);
	std::vector<DataPoint> executeSweep(const Range& omega, Componant *model = nullptr);
	bool executeParamSweep(const std::vector<Range>& componantRanges, const Range& omega, std::function<void(std::vector<DataPoint>&, const std::vector<fvalue>&)> dataCb);
	std::vector<DataPoint> executeParamByIndex(const std::vector<Range>& componantRanges, const Range& omega, size_t index);
	std::vector<DataPoint> executeParamByIndexC(const std::vector<Range>& componantRanges, const Range& omega, size_t index);
	std::string getModelStr();
	std::vector<Componant*> getFlatComponants(Componant *model = nullptr);
	std::vector<fvalue> getFlatParameters(Componant *model = nullptr);
	size_t getFlatParametersCount();
	bool setFlatParameters(const std::vector<fvalue>& parameters, Componant *model = nullptr);
	bool checkParameterRanges(const std::vector<eis::Range> ranges, Componant* model = nullptr);

	static std::vector<fvalue> getSweepParamByIndex(const std::vector<Range>& componantRanges, size_t index);
	static size_t getRequiredStepsForSweeps(const std::vector<Range>& componantRanges);
};

}
