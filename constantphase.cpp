#include "constantphase.h"
#include "tokenize.h"
#include <cstdlib>
#include <math.h>

#include "log.h"

using namespace eis;

Cpe::Cpe(double q, double alpha): _Q(q), _alpha(alpha)
{

}

Cpe::Cpe(std::string paramStr)
{
	std::vector<std::string> tokens = tokenize(paramStr, ',');
	if(tokens.size() < 2)
	{
		std::cout<<"Warning: to few parameters in "<<__func__<<" parameter string: "<<paramStr<<'\n';
		_Q = 1e-7;
		_alpha = 0.9;
		return;
	}
	else
	{
		try
		{
			_Q = std::stod(tokens[0]);
			_alpha = std::stod(tokens[1]);
		}
		catch(const std::invalid_argument& ia)
		{
			std::cout<<"Warning: cant parse parameter in "<<__func__<<" parameter: "<<tokens[0]<<'\n';
			_Q = 1e-7;
			_alpha = 0.9;
		}
	}
}

std::complex<double> Cpe::execute(double omega)
{
	return std::complex<double>((1.0/(_Q*pow(omega, _alpha)))*cos((M_PI/2)*_alpha),
	                           0-(1.0/(_Q*pow(omega, _alpha)))*sin((M_PI/2)*_alpha));
}

std::vector<double> Cpe::getParam()
{
	return std::vector<double>({_Q, _alpha});
}

void Cpe::setParam(const std::vector<double>& param)
{
	if(param.size() < 2)
	{
		std::cout<<"Warning: invalid parameter list sent to "<<__func__<<'\n';
		return;
	}

	_Q = param[0];
	_alpha = param[1];
}

size_t Cpe::paramCount()
{
	return 2;
}
