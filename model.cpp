#include <cstddef>
#include <model.h>
#include <iostream>
#include <assert.h>
#include <vector>
#include <array>
#include <thread>
#include <fstream>

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


Componant *Model::processBrackets(std::string& str, size_t& bracketCounter, size_t paramSweepCount, bool defaultToRange)
{
	size_t bracketStart = deepestBraket(str);
	Log(Log::DEBUG)<<str<<" bracket start "<<(bracketStart == std::string::npos ? std::string("npos") :  std::to_string(bracketStart));

	if(bracketStart == std::string::npos)
	{
		Componant* componant = processBracket(str, paramSweepCount, defaultToRange);
		if(!componant)
			throw parse_errror("can not create componant type B for " + str);
		return componant;
	}

	size_t bracketEnd = opposingBraket(str, bracketStart);

	if(bracketEnd == std::string::npos)
	{
		return nullptr;
	}

	std::string bracket = str.substr(bracketStart+1, bracketEnd-1-bracketStart);

	Componant* componant = processBracket(bracket, paramSweepCount, defaultToRange);
	if(!componant)
		throw parse_errror("can not create componant type B for " + str);
	_bracketComponants.push_back(componant);

	str.erase(str.begin()+bracketStart, str.begin()+bracketEnd+1);
	str.insert(str.begin()+bracketStart, bracketCounter+48);
	++bracketCounter;
	return processBrackets(str, bracketCounter, paramSweepCount, defaultToRange);
}

Componant *Model::processBracket(std::string& str, size_t paramSweepCount, bool defaultToRange)
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
					componants.push_back(new Cap(getParamStr(nodeStr, i), paramSweepCount, defaultToRange));
					i = paramSkipIndex(nodeStr, i);
					break;
				}
				case Resistor::staticGetComponantChar():
				{
					componants.push_back(new Resistor(getParamStr(nodeStr, i), paramSweepCount, defaultToRange));
					i = paramSkipIndex(nodeStr, i);
					break;
				}
				case Inductor::staticGetComponantChar():
				{
					componants.push_back(new Inductor(getParamStr(nodeStr, i), paramSweepCount, defaultToRange));
					i = paramSkipIndex(nodeStr, i);
					break;
				}
				case Cpe::staticGetComponantChar():
				{
					componants.push_back(new Cpe(getParamStr(nodeStr, i), paramSweepCount, defaultToRange));
					i = paramSkipIndex(nodeStr, i);
					break;
				}
				case Warburg::staticGetComponantChar():
				{
					componants.push_back(new Warburg(getParamStr(nodeStr, i), paramSweepCount, defaultToRange));
					i = paramSkipIndex(nodeStr, i);
					break;
				}
				case FiniteTransmitionline::staticGetComponantChar():
				{
					componants.push_back(new FiniteTransmitionline(getParamStr(nodeStr, i), paramSweepCount, defaultToRange));
					i = paramSkipIndex(nodeStr, i);
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

size_t Model::paramSkipIndex(const std::string& str, size_t index)
{
	if(index+2 > str.size() || str[index+1] != '{')
		return index;

	size_t opposing = opposingBraket(str, index, '}');
	if(opposing != std::string::npos)
		return opposing;
	return index;
}

Model::Model(const std::string& str, size_t paramSweepCount, bool defaultToRange): _modelStr(str)
{
	size_t bracketCounter = 0;
	std::string strCpy(str);
	_model = nullptr;
	try
	{
		_model = processBrackets(strCpy, bracketCounter, paramSweepCount, defaultToRange);
	}
	catch(const parse_errror& err)
	{
		Log(Log::ERROR)<<err.what();
		if(_model != nullptr)
			delete _model;
		_model = nullptr;
	}
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
	{
		fvalue omegaStep = omega[i];
		results.push_back(execute(omegaStep, index));
	}

	return results;
}

void Model::sweepThreadFn(std::vector<std::vector<DataPoint>>* data, Model* model, size_t start, size_t stop, const Range& omega)
{
	for(size_t i = start; i < stop; ++i)
	{
		data->at(i) = model->executeSweep(omega, i);
	}
}

std::vector<std::vector<DataPoint>> Model::executeAllSweeps(const Range& omega)
{
	size_t count = getRequiredStepsForSweeps();
	unsigned int threadsCount = std::thread::hardware_concurrency();
	if(count < threadsCount*10)
		threadsCount = 1;
	size_t countPerThread = count/threadsCount;
	std::vector<std::thread> threads(threadsCount);
	std::vector<Model> models(threadsCount, *this);

	std::vector<std::vector<DataPoint>> data(count);

	for(size_t i = 0; i < threadsCount; ++i)
	{
		size_t start = i*countPerThread;
		size_t stop = i < threadsCount-1 ? (i+1)*countPerThread : count;
		threads[i] = std::thread(sweepThreadFn, &data, &models[i], start, stop, std::ref(omega));
	}
	for(size_t i = 0; i < threadsCount; ++i)
		threads[i].join();
	return data;
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

	Log(Log::DEBUG)<<"Magnitudes:";
	for(size_t i = 0; i < flatRanges.size(); ++i)
	{
		size_t magnitude = 1;
		for(int64_t j = static_cast<int64_t>(i)-1; j >= 0; --j)
			magnitude = magnitude*flatRanges[j]->count;
		placeMagnitude.push_back(magnitude);
		Log(Log::DEBUG)<<placeMagnitude.back();
	}

	Log(Log::DEBUG)<<"Steps for index "<<index<<" ranges "<<flatRanges.size()<<" Ranges:";
	for(int64_t i = flatRanges.size()-1; i >= 0; --i)
	{
		flatRanges[i]->step = index/placeMagnitude[i];
		index = index % placeMagnitude[i];
		Log(Log::DEBUG)<<placeMagnitude[i]<<'('<<flatRanges[i]->step<<')'<<(i == 0 ? "" : " + ");
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

std::string Model::getModelStrWithParam(size_t index)
{
	if(!_model)
		return "";
	resolveSteps(index);
	std::string out = _model->getComponantString();
	eisRemoveUnneededBrackets(out);
	return out;
}

std::string Model::getModelStrWithParam() const
{
	if(!_model)
		return "";
	std::string out = _model->getComponantString(false);
	eisRemoveUnneededBrackets(out);
	return out;
}

bool Model::isReady()
{
	return _model;
}

bool Model::isParamSweep()
{
	return getRequiredStepsForSweeps() > 1;
}
