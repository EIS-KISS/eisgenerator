#include <model.h>
#include <iostream>
#include <assert.h>
#include "tokenize.h"
#include "cap.h"
#include "resistor.h"
#include "constantphase.h"
#include "warburg.h"
#include "paralellseriel.h"
#include "log.h"

using namespace eis;

size_t Model::opposingBraket(const std::string& str, size_t index, char bracketChar)
{
	for(size_t i = index; i < str.size(); ++i)
	{
		if(str[i] == bracketChar)
			return i;
	}
	return std::string::npos;
}

size_t Model::deepestBraket(const std::string& str)
{
	size_t deepestPos = std::string::npos;
	size_t deepestLevel = 0;
	size_t level = 0;
	for(size_t i = 0; i < str.size(); ++i)
	{
		if(str[i] == '(')
		{
			++level;
			if(level > deepestLevel)
			{
				deepestLevel = level;
				deepestPos = i;
			}
		}
	}
	return deepestPos;
}

Componant *Model::processBrackets(std::string& str, size_t& bracketCounter)
{
	size_t bracketStart = deepestBraket(str);
	Log(Log::DEBUG)<<str<<" bracket start "<<(bracketStart == std::string::npos ? std::string("npos") :  std::to_string(bracketStart));

	if(bracketStart == std::string::npos)
	{
		Componant* componant = processBracket(str);
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

	Componant* componant = processBracket(bracket);
	if(!componant)
	{
		Log(Log::DEBUG)<<"can not create componant type A for "<<bracket;
	}
	_bracketComponants.push_back(componant);

	str.erase(str.begin()+bracketStart, str.begin()+bracketEnd+1);
	str.insert(str.begin()+bracketStart, bracketCounter+48);
	++bracketCounter;
	return processBrackets(str, bracketCounter);
}

std::string Model::getParamStr(const std::string& str, size_t index)
{
	if(str.size()-index < 3 || str[index+1] != '{')
	{
		Log(Log::DEBUG)<<"missing parameter for "<<str[index];
		return 0;
	}

	size_t end = opposingBraket(str, index, '}');
	std::string parameterStr = str.substr(index+2, end-index-2);
	Log(Log::DEBUG)<<"param for "<<str[index]<<' '<<parameterStr;
	return parameterStr;
}

Componant *Model::processBracket(std::string& str)
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
				case 'c':
				case 'C':
					componants.push_back(new Cap(getParamStr(nodeStr, i)));
					i = opposingBraket(nodeStr, i, '}');
					break;
				case 'r':
				case 'R':
					componants.push_back(new Resistor(getParamStr(nodeStr, i)));
					i = opposingBraket(nodeStr, i, '}');
					break;
				case 'p':
				case 'P':
					componants.push_back(new Cpe(getParamStr(nodeStr, i)));
					i = opposingBraket(nodeStr, i, '}');
					break;
				case 'w':
				case 'W':
					componants.push_back(new Warburg(getParamStr(nodeStr, i)));
					i = opposingBraket(nodeStr, i, '}');
					break;
				case '{':
					i = opposingBraket(nodeStr, i, '}');
				case '}':
					Log(Log::WARN)<<"stray "<<nodeStr[i]<<" in model string";
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

Model::Model(const std::string& str): _modelStr(str)
{
	size_t bracketCounter = 0;
	std::string strCpy(str);
	_model = processBrackets(strCpy, bracketCounter);
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

DataPoint Model::execute(fvalue omega)
{
	if(_model)
	{
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

void Model::addComponantToFlat(Componant* componant)
{
	Parallel* paralell = dynamic_cast<Parallel*>(componant);
	if(paralell)
	{
		for(Componant* element : paralell->componants)
			addComponantToFlat(element);
		return;
	}

	Serial* serial = dynamic_cast<Serial*>(componant);
	if(serial)
	{
		for(Componant* element : serial->componants)
			addComponantToFlat(element);
		return;
	}

	_flatComponants.push_back(componant);
}

std::vector<Componant*> Model::getFlatComponants()
{
	if(!_flatComponants.empty())
		return _flatComponants;

	addComponantToFlat(_model);
	return getFlatComponants();
}

std::vector<DataPoint> Model::executeSweep(const Range& omega)
{
	std::vector<DataPoint> results;
	results.reserve(omega.count);

	if(!omega.log)
	{
		fvalue currOmega = omega.start;
		fvalue step = (omega.end - omega.start)/(omega.count-1);

		for(size_t i = 0; i < omega.count; ++i)
		{
			results.push_back(execute(currOmega));
			currOmega+=step;
		}
	}
	else
	{
		fvalue start = log10(omega.start);
		fvalue end = log10(omega.end);
		fvalue step = (end-start)/(omega.count-1);
		fvalue currOmegaL = start;

		for(size_t i = 0; i < omega.count; ++i)
		{
			results.push_back(execute(pow(10, currOmegaL)));
			currOmegaL+=step;
		}
	}
	return results;
}

std::vector<DataPoint> Model::executeParamByIndex(const std::vector<Range>& componantRanges, const Range& omega, size_t index)
{
	std::vector<fvalue> parameters = getSweepParamByIndex(componantRanges, index);
	assert(setFlatParameters(parameters));

	return executeSweep(omega);
}

size_t Model::getRequiredStepsForSweeps(const std::vector<Range>& componantRanges)
{
	size_t stepsRequired = 1;
	for(size_t i = 0; i < componantRanges.size(); ++i)
		stepsRequired *= componantRanges[i].count;
	return stepsRequired;
}

std::vector<fvalue> Model::getSweepParamByIndex(const std::vector<Range>& componantRanges, size_t index)
{
	size_t parametersCount = componantRanges.size();
	std::vector<size_t> parameterIndexies(parametersCount, 0);
	for(size_t i = 0; i < parametersCount && index > 0; ++i)
	{
		index = i > 0 ? index/componantRanges[i-1].count : index;
		parameterIndexies[i] = index % componantRanges[i].count;
		index -= parameterIndexies[i];
	}

	std::vector<fvalue> parameters(parametersCount, 0);
	for(size_t i = 0; i < parametersCount; ++i)
		parameters[i] = parameterIndexies[i]*componantRanges[i].stepSize()+componantRanges[i].start;
	return parameters;
}

bool Model::executeParamSweep(const std::vector<Range>& componantRanges, const Range& omega,
                              std::function<void(std::vector<DataPoint>&, const std::vector<fvalue>&)> dataCb)
{
	size_t parametersCount = getFlatParametersCount();
	if(componantRanges.size() != parametersCount)
	{
		Log(Log::ERROR)<<"a parameter range must be provided for eatch componant parameter";
		return false;
	}

	for(size_t i = 0; i < parametersCount; ++i)
	{
		if(componantRanges[i].count == 0 || (componantRanges[i].count < 2 && componantRanges[i].start != componantRanges[i].end))
		{
			Log(Log::ERROR)<<"paramter range must specify at least one paramter point if only one paramer point is specified start and end must be the same";
			return false;
		}
		else if(componantRanges[i].start > componantRanges[i].end)
		{
			Log(Log::ERROR)<<"paramter range end-start must be positive";
			return false;
		}
	}

	size_t stepsRequired = getRequiredStepsForSweeps(componantRanges);

	std::vector<fvalue> currentParam(parametersCount, 0);
	for(size_t i = 0; i < parametersCount; ++i)
		currentParam[i] = componantRanges[i].start;

	Log(Log::INFO)<<"Executing sweep. Steps requried: "<<stepsRequired;

	for(size_t i = 0; i < stepsRequired; ++i)
	{
		setFlatParameters(getSweepParamByIndex(componantRanges, i));
		std::vector<DataPoint> result = executeSweep(omega);
		dataCb(result, currentParam);
	}

	return true;
}

bool Model::setFlatParameters(const std::vector<fvalue>& parameters)
{
	if(parameters.size() != getFlatParametersCount())
		return false;

	size_t i = 0;
	for(Componant* componant : getFlatComponants())
	{
		std::vector<fvalue> params;
		for(size_t j = 0; j < componant->paramCount(); ++j)
			params.push_back(parameters[i++]);
		componant->setParam(params);
	}

	return true;
}

std::vector<fvalue> Model::getFlatParameters()
{
	std::vector<fvalue> params;
	std::vector<Componant*> componants = getFlatComponants();
	for(Componant* componant : componants)
	{
		for(fvalue param : componant->getParam())
			params.push_back(param);
	}
	return params;
}

size_t Model::getFlatParametersCount()
{
	size_t count = 0;
	for(Componant* componant : getFlatComponants())
		count += componant->paramCount();
	return count;
}
