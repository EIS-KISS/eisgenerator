#include "resistor.h"
#include "tokenize.h"
#include <vector>
#include <math.h>

Resistor::Resistor(double r): _R(r)
{}

Resistor::Resistor(std::string paramStr)
{
	std::vector<std::string> tokens = tokenize(paramStr, ',');
	if(tokens.empty())
	{
		std::cout<<"Warning: to few parameters in "<<__func__<<" parameter string: "<<paramStr<<'\n';
		_R = 1e3;
		return;
	}
	else
	{
		try
		{
			_R = std::stod(tokens[0]);
		}
		catch(const std::invalid_argument& ia)
		{
			std::cout<<"Warning: cant parse parameter in "<<__func__<<" parameter: "<<tokens[0]<<'\n';
			_R = 1e3;
		}
	}
}

std::complex<double> Resistor::execute(double omega)
{
	(void)omega;
	return std::complex<double>(_R, 0);
}

std::vector<double> Resistor::getParam()
{
	return std::vector<double>({_R});
}

void Resistor::setParam(const std::vector<double>& param)
{
	if(param.empty())
	{
		std::cout<<"Warning: invalid parameter list sent to "<<__func__<<'\n';
		return;
	}

	_R = param[0];
}

size_t Resistor::paramCount()
{
	return 1;
}
