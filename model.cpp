#include <cstddef>
#include <model.h>
#include <iostream>
#include <assert.h>
#include <vector>
#include "strops.h"
#include "cap.h"
#include "resistor.h"
#include "inductor.h"
#include "constantphase.h"
#include "finitetr.h"
#include "warburg.h"
#include "paralellseriel.h"
#include "log.h"

using namespace eis;

Componant *Model::processBrackets(std::string& str, size_t& bracketCounter, size_t paramSweepCount)
{
	size_t bracketStart = deepestBraket(str);
	Log(Log::DEBUG)<<str<<" bracket start "<<(bracketStart == std::string::npos ? std::string("npos") :  std::to_string(bracketStart));

	if(bracketStart == std::string::npos)
	{
		Componant* componant = processBracket(str, paramSweepCount);
		if(!componant)
			Log(Log::DEBUG)<<"Warning: can not create componant type B for "<<str;
		return componant;
	}

	size_t bracketEnd = opposingBraket(str, bracketStart);

	if(bracketEnd == std::string::npos)
	{
		return nullptr;
	}

	std::string bracket = str.substr(bracketStart+1, bracketEnd-1-bracketStart);

	Componant* componant = processBracket(bracket, paramSweepCount);
	if(!componant)
		Log(Log::DEBUG)<<"can not create componant type A for "<<bracket;
	_bracketComponants.push_back(componant);

	str.erase(str.begin()+bracketStart, str.begin()+bracketEnd+1);
	str.insert(str.begin()+bracketStart, bracketCounter+48);
	++bracketCounter;
	return processBrackets(str, bracketCounter, paramSweepCount);
}

std::string Model::getParamStr(const std::string& str, size_t index)
{
	if(static_cast<int64_t>(str.size())-index < 3 || str[index+1] != '{')
	{
		Log(Log::WARN)<<"missing parameter string for "<<str[index];
		return "";
	}

	size_t end = opposingBraket(str, index, '}');
	std::string parameterStr = str.substr(index+2, end-index-2);
	Log(Log::DEBUG)<<"param for "<<str[index]<<' '<<parameterStr;
	return parameterStr;
}

Componant *Model::processBracket(std::string& str, size_t paramSweepCount)
{
	Log(Log::DEBUG)<<__func__<<'('<<str<<')';
	std::vector<std::string> tokens = tokenize(str, '-', '{', '}');

	std::vector<Componant*> nodes;

	for(const std::string& nodeStr : tokens)
	{
		Log(Log::DEBUG)<<__func__<<" full node str: "<<nodeStr;
		std::vector<Componant*> componants;
		for(size_t i = 0; i < nodeStr.size(); ++i)
		{
			Log(Log::DEBUG)<<__func__<<" arg: "<<nodeStr[i];
			switch(nodeStr[i])
			{
				case Cap::staticGetComponantChar():
				{
					componants.push_back(new Cap(getParamStr(nodeStr, i), paramSweepCount));
					size_t opposing = opposingBraket(nodeStr, i, '}');
					if(opposing != std::string::npos)
						i = opposingBraket(nodeStr, i, '}');
					break;
				}
				case Resistor::staticGetComponantChar():
				{
					componants.push_back(new Resistor(getParamStr(nodeStr, i), paramSweepCount));
					size_t opposing = opposingBraket(nodeStr, i, '}');
					if(opposing != std::string::npos)
						i = opposingBraket(nodeStr, i, '}');
					break;
				}
				case Inductor::staticGetComponantChar():
				{
					componants.push_back(new Inductor(getParamStr(nodeStr, i), paramSweepCount));
					size_t opposing = opposingBraket(nodeStr, i, '}');
					if(opposing != std::string::npos)
						i = opposingBraket(nodeStr, i, '}');
					break;
				}
				case Cpe::staticGetComponantChar():
				{
					componants.push_back(new Cpe(getParamStr(nodeStr, i), paramSweepCount));
					size_t opposing = opposingBraket(nodeStr, i, '}');
					if(opposing != std::string::npos)
						i = opposingBraket(nodeStr, i, '}');
					break;
				}
				case Warburg::staticGetComponantChar():
				{
					componants.push_back(new Warburg(getParamStr(nodeStr, i), paramSweepCount));
					size_t opposing = opposingBraket(nodeStr, i, '}');
					if(opposing != std::string::npos)
						i = opposingBraket(nodeStr, i, '}');
					break;
				}
				case FiniteTransmitionline::staticGetComponantChar():
				{
					componants.push_back(new FiniteTransmitionline(getParamStr(nodeStr, i), paramSweepCount));
					size_t opposing = opposingBraket(nodeStr, i, '}');
					if(opposing != std::string::npos)
						i = opposingBraket(nodeStr, i, '}');
					break;
				}
				case '{':
					i = opposingBraket(nodeStr, i, '}');
				case '}':
					Log(Log::WARN)<<getModelStr()<<" stray "<<nodeStr[i]<<" in model string";
					break;
				case '0' ... '9':
				{
					size_t j = nodeStr[i]-48;
					if(_bracketComponants.size() > j)
						componants.push_back(_bracketComponants[j]);
					break;
				}
				default:
					break;
			}
		}
		if(componants.size() > 1)
			nodes.push_back(new Parallel(componants));
		else if(componants.size() == 1)
			nodes.push_back(componants[0]);
		else
			Log(Log::WARN)<<"empty node for "<<nodeStr;
	}

	if(nodes.size() > 1)
		return new Serial(nodes);
	else if(nodes.size() == 1)
		return nodes[0];
	else
		return nullptr;
}

Model::Model(const std::string& str, size_t paramSweepCount): _modelStr(str)
{
	size_t bracketCounter = 0;
	std::string strCpy(str);
	_model = processBrackets(strCpy, bracketCounter, paramSweepCount);
}

Model::Model(const Model& in)
{
	operator=(in);
}

Model& Model::operator=(const Model& in)
{
	delete _model;
	_modelStr = in._modelStr;
	_bracketComponants.clear();
	_flatComponants.clear();
	_model = Componant::copy(in._model);
	return *this;
}

Model::~Model()
{
	delete _model;
}

DataPoint Model::execute(fvalue omega, size_t index)
{
	if(_model)
	{
		resolveSteps(index);
		DataPoint dataPoint;
		dataPoint.omega = omega;
		dataPoint.im = _model->execute(omega);
		return dataPoint;
	}
	else
	{
		Log(Log::WARN)<<"model not ready";
	}
	return DataPoint({std::complex<fvalue>(0,0), 0});
}

void Model::addComponantToFlat(Componant* componant, std::vector<Componant*>* flatComponants)
{
	Parallel* paralell = dynamic_cast<Parallel*>(componant);
	if(paralell)
	{
		for(Componant* element : paralell->componants)
			addComponantToFlat(element, flatComponants);
		return;
	}

	Serial* serial = dynamic_cast<Serial*>(componant);
	if(serial)
	{
		for(Componant* element : serial->componants)
			addComponantToFlat(element, flatComponants);
		return;
	}

	flatComponants->push_back(componant);
}

std::vector<Componant*> Model::getFlatComponants(Componant *model)
{
	if(model == nullptr || model == _model)
	{
		if(!_flatComponants.empty())
			return _flatComponants;

		addComponantToFlat(_model, &_flatComponants);
		return getFlatComponants();
	}
	else
	{
		std::vector<Componant*> flatComponants;
		addComponantToFlat(model, &flatComponants);
		return flatComponants;
	}
}

std::vector<DataPoint> Model::executeSweep(const Range& omega, size_t index)
{
	std::vector<DataPoint> results;
	results.reserve(omega.count);
	for(size_t i = 0; i < omega.count; ++i)
		results.push_back(execute(omega[i], index));

	return results;
}

void Model::resolveSteps(int64_t index)
{
	std::vector<Componant*> componants = getFlatComponants();
	if(index == 0)
	{
		for(Componant* componant : componants)
		{
			for(Range& range :  componant->getParamRanges())
				range.step = 0;
		}
		return;
	}

	assert(static_cast<size_t>(index) < getRequiredStepsForSweeps());

	std::vector<Range*> flatRanges;

	for(Componant* componant : componants)
	{
		for(Range& range :  componant->getParamRanges())
			flatRanges.push_back(&range);
	}

	std::vector<size_t> placeMagnitude;
	placeMagnitude.reserve(flatRanges.size());

	for(size_t i = 0; i < flatRanges.size(); ++i)
	{
		size_t magnitude = 1;
		for(int64_t j = static_cast<int64_t>(i)-1; j >= 0; --j)
		{
			magnitude = magnitude*flatRanges[j]->count;
		}
		placeMagnitude.push_back(magnitude);
	}

	for(int64_t i = flatRanges.size(); i >= 0 && index > 0; --i)
	{
		flatRanges[i]->step = index/placeMagnitude[i];
		index = index % placeMagnitude[i];
		Log(Log::DEBUG)<<placeMagnitude[i]<<'('<<flatRanges[i]->step<<')'<<(i == 0 ? "\n" : " + ");
	}
}

size_t Model::getRequiredStepsForSweeps()
{
	size_t stepsRequired = 1;
	std::vector<Componant*> componants = getFlatComponants();
	for(Componant* componant : componants)
	{
		std::vector<Range> ranges = componant->getParamRanges();
		for(const Range& range : ranges)
			stepsRequired *= range.count;
	}
	return stepsRequired;
}

std::string Model::getModelStr() const
{
	std::string output;
	output.reserve(_modelStr.size());
	int bracket = 0;
	for(const char c : _modelStr)
	{
		if(c == '{')
			++bracket;
		else  if(bracket == 0)
			output.push_back(c);

		if(c == '}')
		{
			--bracket;
			if(bracket < 0)
				return _modelStr;
		}
	}
	return output;
}

bool Model::isParamSweep()
{
	return getRequiredStepsForSweeps() != 0;
}
