#include "cap.h"
#include "tokenize.h"
#include <cstdlib>
#include <math.h>

#include "log.h"

using namespace eis;

Cap::Cap(double c): _C(c)
{

}

Cap::Cap(std::string paramStr)
{
	std::vector<std::string> tokens = tokenize(paramStr, ',');
	if(tokens.empty())
	{
		Log(Log::WARN)<<"to few parameters in "<<__func__<<" parameter string: "<<paramStr<<'\n';
		_C = 1e-6;
		return;
	}
	else
	{
		try
		{
			_C = std::stod(tokens[0]);
		}
		catch(const std::invalid_argument& ia)
		{
			Log(Log::WARN)<<"Warning: cant parse parameter in "<<__func__<<" parameter: "<<tokens[0]<<'\n';
			_C = 1e3;
		}
	}
}

std::complex<double> Cap::execute(double omega)
{
	return std::complex<double>(0, 0.0-(1.0/(_C*omega)));
}

std::vector<double> Cap::getParam()
{
	return std::vector<double>({_C});
}

void Cap::setParam(const std::vector<double>& param)
{
	if(param.empty())
	{
		Log(Log::WARN)<<"invalid parameter list sent to "<<__func__<<'\n';
		return;
	}

	_C = param[0];
}

size_t Cap::paramCount()
{
	return 1;
}
