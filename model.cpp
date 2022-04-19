#include <model.h>
#include <iostream>
#include "tokenize.h"
#include "cap.h"
#include "resistor.h"

Model::Paralell::Paralell(std::vector<Componant*> componantsIn): componants(componantsIn)
{
}

std::complex<double> Model::Paralell::execute(double omega)
{
	std::complex<double> accum(0,0);
	for(Componant* componant : componants)
	{
		accum += std::complex<double>(1,0)/componant->execute(omega);
	}
	return std::complex<double>(1,0)/accum;
}

Model::Serial::Serial(std::vector<Componant*> componantsIn): componants(componantsIn)
{
}

std::complex<double> Model::Serial::execute(double omega)
{
	std::complex<double> accum(0,0);
	for(Componant* componant : componants)
	{
		accum += componant->execute(omega);
	}
	return accum;
}

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
			nodes.push_back(new Paralell(componants));
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
