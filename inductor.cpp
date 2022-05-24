#include "inductor.h"
#include "tokenize.h"
#include <cstdlib>
#include <math.h>

#include "log.h"

using namespace eis;

Inductor::Inductor(fvalue L): _L(L)
{

}

Inductor::Inductor(std::string paramStr)
{
	std::vector<std::string> tokens = tokenize(paramStr, ',');
	if(tokens.empty())
	{
		Log(Log::WARN)<<"to few parameters in "<<__func__<<" parameter string: "<<paramStr<<'\n';
		_L = 1e-6;
		return;
	}
	else
	{
		try
		{
			_L = std::stod(tokens[0]);
		}
		catch(const std::invalid_argument& ia)
		{
			Log(Log::WARN)<<"Warning: cant parse parameter in "<<__func__<<" parameter: "<<tokens[0]<<'\n';
			_L = 1e3;
		}
	}
}

std::complex<fvalue> Inductor::execute(fvalue omega)
{
	return std::complex<fvalue>(0, _L*omega);
}

std::vector<fvalue> Inductor::getParam()
{
	return std::vector<fvalue>({_L});
}

void Inductor::setParam(const std::vector<fvalue>& param)
{
	if(param.empty())
	{
		Log(Log::WARN)<<"invalid parameter list sent to "<<__func__<<'\n';
		return;
	}

	_L = param[0];
}

size_t Inductor::paramCount()
{
	return 1;
}

char Inductor::getComponantChar() const
{
	return staticGetComponantChar();
}
