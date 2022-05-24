#include "finitetr.h"
#include <cstdlib>
#include <math.h>

#include "eisgenerator/componant.h"
#include "eisgenerator/paralellseriel.h"
#include "eisgenerator/resistor.h"
#include "paralellseriel.h"
#include "cap.h"
#include "resistor.h"
#include "log.h"
#include "tokenize.h"

using namespace eis;

FiniteTransmitionline::FiniteTransmitionline(fvalue c, fvalue r, unsigned int n): _C(c), _R(r), _n(n)
{
	if(n < 1)
	{
		Log(Log::WARN)<<__func__<<" n must be > 0 setting n to 4";
		_n = 4;
	}

	subComponant = createTransmitionLine(_C, _R, _n);
}

FiniteTransmitionline::FiniteTransmitionline(std::string paramStr)
{
	std::vector<std::string> tokens = tokenize(paramStr, ',');
	if(tokens.size() < paramCount())
	{
		Log(Log::WARN)<<"to few parameters in "<<__func__<<" parameter string: "<<paramStr;
		_C = 1e-6;
		_R = 1000;
		_n = 4;
		subComponant = createTransmitionLine(_C, _R, _n);
		return;
	}
	else
	{
		try
		{
			_R = std::stod(tokens[0]);
			_C = std::stod(tokens[1]);
			_n = std::stod(tokens[2]);
			subComponant = createTransmitionLine(_C, _R, _n);
		}
		catch(const std::invalid_argument& ia)
		{
			Log(Log::WARN)<<"Warning: cant parse parameter in "<<__func__<<" parameter: "<<tokens[0]<<'\n';
			_C = 1e3;
		}
	}
}

FiniteTransmitionline::FiniteTransmitionline(const FiniteTransmitionline& in)
{
	_R = in._R;
	_C = in._C;
	_n = in._n;
	subComponant = createTransmitionLine(_C, _R, _n);
}

std::complex<fvalue> FiniteTransmitionline::execute(fvalue omega)
{
	if(subComponant)
	{
		return subComponant->execute(omega);
	}
	else
	{
		Log(Log::WARN)<<"Invalid transmitionline used";
		return std::complex<fvalue>(1, 0);
	}
}

std::vector<fvalue> FiniteTransmitionline::getParam()
{
	return std::vector<fvalue>({_R, _C, static_cast<fvalue>(_n)});
}

void FiniteTransmitionline::setParam(const std::vector<fvalue>& param)
{
	if(param.size() < paramCount())
	{
		Log(Log::WARN)<<"invalid parameter list sent to "<<__func__<<'\n';
		return;
	}

	_R = param[0];
	_C = param[1];
	_n = param[2];

	if(param[2] < 1)
	{
		Log(Log::WARN)<<"invalid parameter list sent to "<<__func__<<" n must be > 1 "<<'\n';
		return;
	}

	_n = param[2];
}

char FiniteTransmitionline::getComponantChar() const
{
	return FiniteTransmitionline::staticGetComponantChar();
}

size_t FiniteTransmitionline::paramCount()
{
	return 3;
}

Componant* FiniteTransmitionline::createTransmitionLine(fvalue c, fvalue r, unsigned int n)
{
	Parallel* par = new Parallel;
	par->componants.push_back(new Cap(c));
	par->componants.push_back(new Resistor(r));

	for(unsigned int i = 0; i < n; ++i)
	{
		Serial* ser = new Serial;
		ser->componants.push_back(new Resistor(r));
		ser->componants.push_back(par);

		par = new Parallel;
		par->componants.push_back(ser);
		par->componants.push_back(new Cap(c));
	}

	return par;
}

FiniteTransmitionline::~FiniteTransmitionline()
{
	if(subComponant)
		delete subComponant;
}
