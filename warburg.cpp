#include "warburg.h"
#include "tokenize.h"
#include <cstdlib>
#include <math.h>

#include "log.h"

using namespace eis;

Warburg::Warburg(fvalue a): _A(a)
{

}

Warburg::Warburg(std::string paramStr)
{
	std::vector<std::string> tokens = tokenize(paramStr, ',');
	if(tokens.size() < 1)
	{
		Log(Log::WARN)<<"to few parameters in "<<__func__<<" parameter string: "<<paramStr;
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
			Log(Log::WARN)<<"can't parse parameter in "<<__func__<<" parameter: "<<tokens[0]<<'\n';
		}
	}
}

std::complex<fvalue> Warburg::execute(fvalue omega)
{
	fvalue N = _A/(sqrt(omega));
	return std::complex<fvalue>(N, 0-N);
}

std::vector<fvalue> Warburg::getParam()
{
	return std::vector<fvalue>({_A});
}

void Warburg::setParam(const std::vector<fvalue>& param)
{
	if(param.size() != paramCount())
	{
		Log(Log::WARN)<<"Warning: invalid parameter list sent to "<<__func__<<'\n';
		return;
	}

	_A = param[0];
}

size_t Warburg::paramCount()
{
	return 1;
}

char Warburg::getComponantChar() const
{
	return staticGetComponantChar();
}
