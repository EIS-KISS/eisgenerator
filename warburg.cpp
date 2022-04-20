#include "warburg.h"
#include "tokenize.h"
#include <cstdlib>
#include <math.h>

Warburg::Warburg(double a): _A(a)
{

}

Warburg::Warburg(std::string paramStr)
{
	std::vector<std::string> tokens = tokenize(paramStr, ',');
	if(tokens.size() < 1)
	{
		std::cout<<"Warning: to few parameters in "<<__func__<<" parameter string: "<<paramStr<<'\n';
		return;
	}
	else
	{
		try
		{
			_A = std::stod(tokens[0]);
		}
		catch(const std::invalid_argument& ia)
		{
			std::cout<<"Warning: cant parse parameter in "<<__func__<<" parameter: "<<tokens[0]<<'\n';
		}
	}
}

std::complex<double> Warburg::execute(double omega)
{
	double N = _A/(sqrt(omega));
	return std::complex<double>(N, 0-N);
}

std::vector<double> Warburg::getParam()
{
	return std::vector<double>({_A});
}

void Warburg::setParam(const std::vector<double>& param)
{
	if(param.size() != paramCount())
	{
		std::cout<<"Warning: invalid parameter list sent to "<<__func__<<'\n';
		return;
	}

	_A = param[0];
}

size_t Warburg::paramCount()
{
	return 1;
}
