#include <model.h>
#include <iostream>
#include <assert.h>
#include "tokenize.h"
#include "cap.h"
#include "resistor.h"
#include "constantphase.h"
#include "warburg.h"
#include "paralellseriel.h"

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
	std::cout<<str<<" bracket start "<<(bracketStart == std::string::npos ? std::string("npos") :  std::to_string(bracketStart))<<'\n';

	if(bracketStart == std::string::npos)
	{
		Componant* componant = processBracket(str);
		if(!componant)
		{
			std::cout<<"Warning: can not create componant type B for "<<str<<'\n';
		}
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
		std::cout<<"Warning: can not create componant type A for "<<bracket<<'\n';
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
		std::cout<<"Warning: missing parameter for "<<str[index]<<'\n';
		return 0;
	}

	size_t end = opposingBraket(str, index, '}');
	std::string parameterStr = str.substr(index+2, end-index-2);
	std::cout<<"param for "<<str[index]<<' '<<parameterStr<<'\n';
	return parameterStr;
}

Componant *Model::processBracket(std::string& str)
{
	std::cout<<__func__<<'('<<str<<")\n";
	std::vector<std::string> tokens = tokenize(str, '-', '{', '}');

	std::vector<Componant*> nodes;

	for(const std::string& nodeStr : tokens)
	{
		std::cout<<__func__<<" full node str: "<<nodeStr<<'\n';
		std::vector<Componant*> componants;
		for(size_t i = 0; i < nodeStr.size(); ++i)
		{
			std::cout<<__func__<<" arg: "<<nodeStr[i]<<'\n';
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
					std::cout<<"Warning: stray "<<nodeStr[i]<<" in model string\n";
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
			std::cout<<"Warning: empty node for "<<nodeStr<<'\n';
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

Model::DataPoint Model::execute(double omega)
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
		std::cout<<"Warning: model not ready\n";
	}
	return DataPoint({std::complex<double>(0,0), 0});
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

std::vector<Model::DataPoint> Model::sweep(const Range& omega)
{
	double step = (omega.end - omega.start)/omega.count;
	double currOmega = omega.start;

	std::vector<DataPoint> results;
	results.reserve(omega.count);
	for(size_t i = 0; i < omega.count; ++i)
	{
		results.push_back(execute(currOmega));
		currOmega+=step;
	}
	return results;
}

bool Model::sweepParams(const std::vector<Range>& componantRanges, const Range& omega, std::function<void(std::vector<DataPoint>&, const std::vector<double>&)> dataCb)
{
	size_t parametersCount = getFlatParametersCount();
	if(componantRanges.size() != parametersCount)
	{
		std::cout<<"Error: a parameter range must be provided for eatch componant parameter\n";
		return false;
	}

	for(size_t i = 0; i < parametersCount; ++i)
	{
		if(componantRanges[i].count == 0 || (componantRanges[i].count < 2 && componantRanges[i].start != componantRanges[i].end))
		{
			std::cout<<"Error: paramter range must specify at least one paramter point if only one paramer point is specified star and end must be the same\n";
			return false;
		}
		else if(componantRanges[i].start > componantRanges[i].end)
		{
			std::cout<<"Error: paramter range end-start must be positive\n";
			return false;
		}
	}

	size_t stepsRequired = 1;
	for(size_t i = 0; i < parametersCount; ++i)
		stepsRequired*=componantRanges[i].count;

	std::vector<double> currentParam(parametersCount, 0);
	std::vector<double> stepSize(parametersCount, 0);
	for(size_t i = 0; i < parametersCount; ++i)
	{
		currentParam[i] = componantRanges[i].start;
		stepSize[i] = (componantRanges[i].end - componantRanges[i].start)/componantRanges[i].count;
	}

	std::cout<<"Executing sweep. Steps requried: "<<stepsRequired<<std::endl;

	for(size_t i = 0; i < stepsRequired; ++i)
	{
		for(size_t i = 0; i < parametersCount; ++i)
		{
			currentParam[i] += stepSize[i];
			if(currentParam[i] > componantRanges[i].end)
				currentParam[i] = componantRanges[i].start;
			else
				break;
		}
		std::vector<DataPoint> result = sweep(omega);
		dataCb(result, currentParam);
	}

	return true;
}

bool Model::setFlatParameters(const std::vector<double>& parameters)
{
	if(parameters.size() != getFlatParametersCount())
		return false;

	size_t i = 0;
	for(Componant* componant : getFlatComponants())
	{
		std::vector<double> params;
		for(size_t j = 0; j < componant->paramCount(); ++j)
			params.push_back(parameters[i++]);
		componant->setParam(params);
	}

	return true;
}

size_t Model::getFlatParametersCount()
{
	size_t count = 0;
	for(Componant* componant : getFlatComponants())
		count += componant->paramCount();
	return count;
}

