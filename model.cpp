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

size_t Model::opposingBraket(const std::string& str, size_t index)
{
	for(size_t i = index; i < str.size(); ++i)
	{
		if(str[i] == ')')
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

Componant *Model::processBrackets(std::string& str, size_t& bracketCounter, const std::vector<double>& param)
{
	size_t bracketStart = deepestBraket(str);
	std::cout<<str<<" bracket start "<<(bracketStart == std::string::npos ? std::string("npos") :  std::to_string(bracketStart))<<'\n';

	if(bracketStart == std::string::npos)
	{
		Componant* componant = processBracket(str, param);
		if(!componant)
		{
			std::cout<<"Warning: can not create componant for "<<str;
		}
		return processBracket(str, param);
	}

	size_t bracketEnd = opposingBraket(str, bracketStart);

	if(bracketEnd == std::string::npos)
	{
		return nullptr;
	}

	std::string bracket = str.substr(bracketStart+1, bracketEnd-1);

	Componant* componant = processBracket(bracket, param);
	_bracketComponants.push_back(processBracket(bracket, param));
	if(!componant)
	{
		std::cout<<"Warning: can not create componant for "<<bracket;
	}

	str.erase(str.begin()+bracketStart, str.begin()+bracketEnd+1);
	str.insert(str.begin()+bracketStart, bracketCounter+48);
	++bracketCounter;
	return processBrackets(str, bracketCounter, param);
}

Componant *Model::processBracket(std::string& str, const std::vector<double>& param)
{
	std::vector<std::string> tokens = tokenize(str, '-');

	std::vector<Componant*> nodes;

	size_t paramCounter = 0;

	for(const std::string& nodeStr : tokens)
	{
		std::vector<Componant*> componants;
		for(char c : nodeStr)
		{
			if(paramCounter >= param.size())
				break;
			switch(c)
			{
				case 'c':
				case 'C':
					componants.push_back(new Cap(1e-6));
					break;
				case 'r':
				case 'R':
					componants.push_back(new Resistor(1e3));
					break;
				case '0' ... '9':
				{
					size_t i = c-48;
					if(_bracketComponants.size() > i)
						componants.push_back(_bracketComponants[i]);
					break;
				}
				default:
					break;
			}
		}
		nodes.push_back(new Paralell(componants));
	}

	if(nodes.size() > 1)
		return new Serial(nodes);
	else if(nodes.size() == 1)
		return nodes[0];
	else
		return nullptr;
}

Model::Model(const std::string& str, const std::vector<double>& param)
{
	size_t bracketCounter = 0;
	std::string strCpy(str);
	_model = processBrackets(strCpy, bracketCounter, param);
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
		std::cout<<"Warning: model not ready\n";
	return DataPoint({std::complex<double>(0,0), 0});
}
