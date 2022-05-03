#include "resistor.h"
#include "tokenize.h"
#include <vector>
#include <math.h>

#include "log.h"

using namespace eis;

Resistor::Resistor(fvalue r): _R(r)
{}

Resistor::Resistor(std::string paramStr)
{
	std::vector<std::string> tokens = tokenize(paramStr, ',');
	if(tokens.empty())
	{
		Log(Log::WARN)<<"to few parameters in "<<__func__<<" parameter string: "<<paramStr;
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
			Log(Log::WARN)<<"can't parse parameter in "<<__func__<<" parameter: "<<tokens[0];
			_R = 1e3;
		}
	}
}

std::complex<fvalue> Resistor::execute(fvalue omega)
{
	(void)omega;
	return std::complex<fvalue>(_R, 0);
}

std::vector<fvalue> Resistor::getParam()
{
	return std::vector<fvalue>({_R});
}

void Resistor::setParam(const std::vector<fvalue>& param)
{
	if(param.empty())
	{
		Log(Log::WARN)<<"invalid parameter list sent to "<<__func__;
		return;
	}

	_R = param[0];
}

size_t Resistor::paramCount()
{
	return 1;
}
