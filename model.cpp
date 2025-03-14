//SPDX-License-Identifier:         LGPL-3.0-or-later
//
// eisgenerator - a shared libary and application to generate EIS spectra
// Copyright (C) 2022-2024 Carl Philipp Klemm <carl@uvos.xyz>
//
// This file is part of eisgenerator.
//
// eisgenerator is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eisgenerator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with eisgenerator.  If not, see <http://www.gnu.org/licenses/>.
//

#include <cstddef>
#include <model.h>
#include <iostream>
#include <assert.h>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <algorithm>
#include <execution>
#include <dlfcn.h>
#include <functional>

#include "componant/componant.h"
#include "componant/resistor.h"
#include "strops.h"
#include "componant/paralellseriel.h"
#include "log.h"
#include "normalize.h"
#include "basicmath.h"
#include "compile.h"
#include "compcache.h"

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
			if(Componant::isValidComponantChar(nodeStr[i]))
			{
				componants.push_back(Componant::createNewComponant(nodeStr[i], getParamStr(nodeStr, i), paramSweepCount, defaultToRange));
				i = paramSkipIndex(nodeStr, i);
			}
			else
			{
				switch(nodeStr[i])
				{
					case '{':
						i = opposingBraket(nodeStr, i, '}');
					case '}':
						Log(Log::WARN)<<getModelStr()<<" stray "<<nodeStr[i]<<" in model string";
						break;
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					{
						size_t j = nodeStr[i]-48;
						if(_bracketComponants.size() > j)
							componants.push_back(_bracketComponants[j]);
						break;
					}
					default:
						Log(Log::DEBUG)<<"Invalid char "<<nodeStr[i]<<" in model string";
						return nullptr;
				}
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
	_model = processBrackets(strCpy, bracketCounter, paramSweepCount, defaultToRange);
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
	_compiledModel = in._compiledModel;
	return *this;
}

Model::~Model()
{
	delete _model;
}


std::vector<Range> Model::getFlatParameterRanges()
{
	std::vector<Componant*> flatComponants = getFlatComponants();
	std::vector<Range> out;
	out.reserve(getParameterCount());
	for(Componant* componant : flatComponants)
	{
		const std::vector<Range> ranges = componant->getParamRanges();
		for(const Range& range : ranges)
			out.push_back(range);
	}
	return out;
}

std::vector<fvalue> Model::getFlatParameters()
{
	std::vector<Range> flatRanges = getFlatParameterRanges();

	std::vector<fvalue> out;
	out.reserve(flatRanges.size());
	for(const Range& range : flatRanges)
		out.push_back(range.stepValue());
	return out;
}

std::vector<Range> Model::getDefaultParameters()
{
	std::vector<Componant*> flatComponants = getFlatComponants();

	std::vector<Range> out;
	out.reserve(getParameterCount());
	for(Componant* componant : flatComponants)
	{
		const std::vector<Range> ranges = componant->getDefaultParameters(true);
		for(const Range& range : ranges)
			out.push_back(range);
	}
	return out;
}

std::vector<std::string> Model::getParameterNames()
{
	std::vector<Componant*> flatComponants = getFlatComponants();

	std::vector<std::string> out;
	out.reserve(getParameterCount());
	for(Componant* componant : flatComponants)
	{
		for(size_t i = 0; i < componant->paramCount(); ++i)
			out.push_back(componant->getUniqueName() + "_" + std::to_string(i));
	}

	return out;
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

size_t Model::setParamSweepCountClosestTotal(size_t totalCount)
{
	size_t activeParams = getActiveParameterCount();
	if(activeParams < 1)
	{
		Log(Log::WARN)<<getModelStr()<<" requested "<<totalCount<<
		" param sweep steps from this model, but this model has no active parameters, ignoreing request";
		return 1;
	}
	size_t countPerParam = std::pow(totalCount, 1.0/activeParams);
	for(Componant* componant : getFlatComponants())
	{
		for(eis::Range& range : componant->getParamRanges())
		{
			if(range.count > 1)
				range.count = countPerParam;
		}
	}
	return std::pow(countPerParam, activeParams);
}

size_t Model::getParameterCount()
{
	size_t count = 0;
	for(Componant* componant : getFlatComponants())
		count += componant->paramCount();
	return count;
}

size_t Model::getActiveParameterCount()
{
	size_t count = 0;
	for(Componant* componant : getFlatComponants())
	{
		for(const eis::Range& range : componant->getParamRanges())
		{
			if(range.count > 1)
				++count;
		}
	}
	return count;
}

std::vector<DataPoint> Model::executeSweep(const Range& omega, size_t index)
{
	return executeSweep(omega.getRangeVector(), index);
}

std::vector<DataPoint> Model::executeSweep(const std::vector<fvalue>& omega, size_t index)
{
	std::vector<DataPoint> results;
	results.reserve(omega.size());

	if(_compiledModel)
	{
		resolveSteps(index);
		std::vector<fvalue> parameters = getFlatParameters();
		std::vector<std::complex<fvalue>> values = _compiledModel->symbol(parameters, omega);
		for(size_t i = 0; i < omega.size(); ++i)
		{
			DataPoint dataPoint;
			dataPoint.omega = omega[i];
			dataPoint.im = values[i];
			results.push_back(dataPoint);
		}
	}
	else
	{
		for(size_t i = 0; i < omega.size(); ++i)
		{
			fvalue omegaStep = omega[i];
			results.push_back(execute(omegaStep, index));
		}
	}
	return results;
}

std::vector<std::vector<DataPoint>> Model::executeSweeps(const Range& omega, const std::vector<size_t>& indecies, bool parallel)
{
	return executeSweeps(omega.getRangeVector(), indecies, parallel);
}

std::vector<std::vector<DataPoint>> Model::executeSweeps(const std::vector<fvalue>& omega, const std::vector<size_t>& indecies, bool parallel)
{
	unsigned int threadsCount = parallel ? std::thread::hardware_concurrency() : 1;

	if(indecies.size() < threadsCount*10)
		threadsCount = 1;

	size_t countPerThread = indecies.size()/threadsCount;
	std::vector<std::thread> threads(threadsCount);
	std::vector<Model> models(threadsCount, *this);

	std::vector<std::vector<DataPoint>> data(indecies.size());

	for(size_t i = 0; i < threadsCount; ++i)
	{
		size_t start = i*countPerThread;
		size_t stop = i < threadsCount-1 ? (i+1)*countPerThread : indecies.size();
		threads[i] = std::thread(sweepThreadFn, &data, &models[i], start, stop, omega);
	}
	for(size_t i = 0; i < threadsCount; ++i)
		threads[i].join();

	return data;
}

void Model::sweepThreadFn(std::vector<std::vector<DataPoint>>* data, Model* model, size_t start, size_t stop, const std::vector<fvalue>& omega)
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
		threads[i] = std::thread(sweepThreadFn, &data, &models[i], start, stop, omega.getRangeVector());
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

	std::vector<Range*> flatRanges;

	for(Componant* componant : componants)
	{
		for(Range& range :  componant->getParamRanges())
			flatRanges.push_back(&range);
	}

	std::vector<size_t> placeMagnitude;
	placeMagnitude.reserve(flatRanges.size());

	//Log(Log::DEBUG)<<"Magnitudes:";
	for(size_t i = 0; i < flatRanges.size(); ++i)
	{
		size_t magnitude = 1;
		for(int64_t j = static_cast<int64_t>(i)-1; j >= 0; --j)
			magnitude = magnitude*flatRanges[j]->count;
		placeMagnitude.push_back(magnitude);
		//Log(Log::DEBUG)<<placeMagnitude.back();
	}

	//Log(Log::DEBUG)<<"Steps for index "<<index<<" ranges "<<flatRanges.size()<<" Ranges:";
	for(int64_t i = flatRanges.size()-1; i >= 0; --i)
	{
		flatRanges[i]->step = index/placeMagnitude[i];
		index = index % placeMagnitude[i];
		//Log(Log::DEBUG)<<placeMagnitude[i]<<'('<<flatRanges[i]->step<<')'<<(i == 0 ? "" : " + ");
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

std::vector<size_t> Model::getRecommendedParamIndices(eis::Range omegaRange, double distance, bool threaded)
{
	std::vector<std::vector<eis::DataPoint>> sweeps;
	size_t count = getRequiredStepsForSweeps();
	eis::Log(eis::Log::INFO)<<"Executeing "<<count<<" steps";
	std::vector<std::vector<eis::DataPoint>> allSweeps;
	std::vector<size_t> indices;

	if(threaded)
		allSweeps = executeAllSweeps(omegaRange);

	for(size_t i = 0; i < count; ++i)
	{
		std::vector<eis::DataPoint> data;
		if(threaded)
			data = allSweeps[i];
		else
			data = executeSweep(omegaRange, i);
		normalize(data);

		fvalue maxJump =  maximumNyquistJump(data);
		if(maxJump > 0.30)
		{
			eis::Log(eis::Log::DEBUG)<<"skipping output for step "<<i
				<<" is not well centered: "<<maxJump;
			continue;
		}

		fvalue correlation = std::abs(pearsonCorrelation(data));
		if(correlation > 0.8)
		{
			eis::Log(eis::Log::DEBUG)<<"skipping output for step "<<i
				<<" as data is too linear: "<<correlation;
			continue;
		}

		std::vector<std::vector<eis::DataPoint>>::iterator search;
		if(threaded)
		{
			search = std::find_if(std::execution::par, sweeps.begin(), sweeps.end(),
							[distance, &data](std::vector<eis::DataPoint>& a){return distance > eisDistance(data, a);});
		}
		else
		{
			search = std::find_if(std::execution::seq, sweeps.begin(), sweeps.end(),
							[distance, &data](std::vector<eis::DataPoint>& a){return distance > eisDistance(data, a);});
		}
		if(search == sweeps.end())
		{
			indices.push_back(i);
			sweeps.push_back(data);
			if(threaded)
				resolveSteps(i);
		}
		if(i % 200 == 0)
		{
			eis::Log(eis::Log::INFO, false)<<'.';
			std::cout<<std::flush;
		}
	}

	std::vector<size_t> out;
	out.reserve(indices.size());
	for(size_t candidate : indices)
	{
		resolveSteps(candidate);
		if(!allElementsContribute(omegaRange))
			eis::Log(eis::Log::DEBUG)<<"skipping "<<candidate<<" as not all elements contribute";
		else if(!hasSeriesDifference(omegaRange))
			eis::Log(eis::Log::DEBUG)<<"skipping "<<candidate<<" as not all elements in series are different";
		else
			out.push_back(candidate);

	}

	eis::Log(eis::Log::INFO, false)<<'\n';
	return out;
}

size_t Model::getUuid() const
{
	return std::hash<std::string>{}(getModelStr());
}

bool Model::compile()
{
	if(!_model->compileable())
	{
		Log(Log::WARN)<<"This model can not be compiled, because it contains "
			<<"componants that lack a compiled reprisentation, expect performance degredation";
		return false;
	}

	CompCache* cache = CompCache::getInstance();

	_compiledModel = cache->getObject(getUuid());
	if(!_compiledModel)
	{
		std::filesystem::path tmp = getTempdir();
		size_t uuid = getUuid();

		std::filesystem::path path = tmp/(std::to_string(getUuid())+".so");
		int ret = compile_code(getCode(), path.string());
		if(ret != 0)
		{
			Log(Log::WARN)<<"Unable to compile model!! expect performance degredation";
			return false;
		}

		CompiledObject object;
		object.objectCode = dlopen(path.string().c_str(), RTLD_NOW);
		if(!object.objectCode)
			throw std::runtime_error("Unable to dlopen compiled model " + std::string(dlerror()));

		std::string symbolName = getCompiledFunctionName();
		object.symbol =
			reinterpret_cast<std::vector<std::complex<fvalue>>(*)(const std::vector<fvalue>&, const std::vector<fvalue>&)>
				(dlsym(object.objectCode, symbolName.c_str()));

		if(!object.symbol)
			throw std::runtime_error(path.string() + " dosent have a symbol " + symbolName);

		cache->addObject(uuid, object);
		_compiledModel = cache->getObject(uuid);
	}

	return true;
}

void Model::dropCompiled()
{
	_compiledModel = nullptr;
}

std::string Model::getCode()
{
	if(!_model || !_model->compileable())
		return "";

	std::vector<std::string> parameters;
	std::string formular = _model->getCode(parameters);

	std::string out =
	"#include <cmath>\n"
	"#include <cassert>\n"
	"#include <vector>\n"
	"#include <complex>\n\n"
	"typedef float fvalue;\n\n"
	"extern \"C\"\n{\n\n"
	"std::vector<std::complex<fvalue>> ";
	out.append(getCompiledFunctionName());
	out.append("(const std::vector<fvalue>& parameters, const std::vector<fvalue> omegas)\n{\n\tassert(parameters.size() == ");
	out.append(std::to_string(parameters.size()));
	out.append(");\n\n");
	out.append("\tstd::vector<std::complex<fvalue>> out(omegas.size());\n");

	for(size_t i = 0; i < parameters.size(); ++i)
		out.append("\tfvalue " + parameters[i] + " = parameters[" + std::to_string(i) +  "];\n");

	out.append("\tfor(size_t i = 0; i < omegas.size(); ++i)\n\t{\n");
	out.append("\t\tconst fvalue& omega = omegas[i];\n");
	out.append("\t\tout[i] = ");
	out.append(formular);
	out.append(";\n\t}\n\treturn out;\n}\n\n}\n");
	return out;
}

std::string Model::getTorchScript()
{
	if(!_model || !_model->compileable())
		return "";

	std::vector<std::string> parameters;
	std::string formular = _model->getTorchScript(parameters);

	std::stringstream out;
	out<<"def "<<getCompiledFunctionName()<<"(parameters: torch.Tensor, omegas: torch.Tensor) -> torch.Tensor:\n";
	out<<"    assert parameters.size(0) == "<<parameters.size()<<"\n\n";
	for(size_t i = 0; i < parameters.size(); ++i)
		out<<"    "<<parameters[i]<<" = parameters["<<i<<"]\n";
	out<<"\n    return "<<formular<<"+0*omegas\n";
	return out.str();
}

std::string Model::getCompiledFunctionName() const
{
	return "model_"+std::to_string(getUuid());
}

void Model::removeSeriesResitance(std::string& model)
{
	for(size_t i = 0; i < model.size(); ++i)
	{
		if(model[i] == Resistor::staticGetComponantChar())
		{
			if((i == 0 || model[i-1] == '-') && (i == model.size()-1 || model[i+1] == '-' || model[i+1] == '{'))
			{
				if(i != model.size()-1 && model[i+1] == '{')
				{
					size_t close = opposingBraket(model, i+1, getOpposingBracketChar(model[i+1]));
					if(close == model.size()-1 || model[close+1] == '-')
					{
						eraseModelElement(model, i);
						i = 0;
					}
				}
				else
				{
					eraseModelElement(model, i);
					i = 0;
				}
			}
		}
		else if(model[i] == '(')
		{
			i = opposingBraket(model, i, getOpposingBracketChar(model[i]));
		}
	}
}

bool Model::allElementsContribute(eis::Range omegaRange, fvalue threashold)
{
	std::vector<Componant*> componants = getFlatComponants();
	componants.push_back(_model);
	std::vector<ParallelSerial*> combiners;
	for(Componant* componant : componants)
	{
		ParallelSerial* candidate = dynamic_cast<ParallelSerial*>(componant);
		if(candidate)
			combiners.push_back(candidate);
	}

	std::vector<bool> contributesGlobal;
	for(fvalue omega : omegaRange.getRangeVector())
	{
		std::vector<bool> contributes;
		for(ParallelSerial* combiner : combiners)
		{
			std::vector<bool> contributesLocal = combiner->contributes(omega);
			contributes.insert(contributes.end(), contributesLocal.begin(), contributesLocal.end());
		}
		if(contributesGlobal.empty())
		{
			contributesGlobal = contributes;
		}
		else
		{
			for(size_t i = 0; i < contributesGlobal.size(); ++i)
				contributesGlobal[i] = contributes[i] || contributesGlobal[i];
		}

		if(std::find(contributesGlobal.begin(), contributesGlobal.end(), false) == contributesGlobal.end())
			break;
	}

	return std::find(contributesGlobal.begin(), contributesGlobal.end(), false) == contributesGlobal.end();
}


bool Model::hasSeriesDifference(eis::Range omegaRange, fvalue threashold)
{
	std::vector<Componant*> componants = getFlatComponants();
	componants.push_back(_model);
	std::vector<Serial*> serials;
	for(Componant* componant : componants)
	{
		Serial* candidate = dynamic_cast<Serial*>(componant);
		if(candidate)
			serials.push_back(candidate);
	}

	std::vector<bool> isDifferentGlobal;
	for(fvalue omega : omegaRange.getRangeVector())
	{
		bool allContribute = true;
		for(Serial* serial : serials)
		{
			std::vector<std::complex<fvalue>> impedances;
			for(Componant* componant : serial->componants)
				impedances.push_back(componant->execute(omega));
			for(size_t i = 0; i < impedances.size() && allContribute; ++i)
			{
				for(size_t j = i+1; j < impedances.size(); ++j)
				{
					if(std::abs(impedances[i] - impedances[j])/std::abs(impedances[i]) < threashold)
					{
						allContribute = false;
						break;
					}
				}
			}
			if(!allContribute)
				break;
		}
		if(allContribute)
			return true;
	}

	return false;
}
