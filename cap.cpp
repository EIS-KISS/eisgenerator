#include "cap.h"
#include "strops.h"
#include <cstdlib>
#include <math.h>

#include "log.h"

using namespace eis;

Cap::Cap(fvalue c): _C(c)
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

std::complex<fvalue> Cap::execute(fvalue omega)
{
	return std::complex<fvalue>(0, 0.0-(1.0/(_C*omega)));
}

std::vector<fvalue> Cap::getParam()
{
	return std::vector<fvalue>({_C});
}

void Cap::setParam(const std::vector<fvalue>& param)
{
	if(param.empty())
	{
		Log(Log::WARN)<<"invalid parameter list sent to "<<__func__<<'\n';
		return;
	}

	_C = param[0];
}

char Cap::getComponantChar() const
{
	return Cap::staticGetComponantChar();
}

size_t Cap::paramCount()
{
	return 1;
}
