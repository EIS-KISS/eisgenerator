#include "paralellseriel.h"

using namespace eis;

Parallel::Parallel(std::vector<Componant*> componantsIn): componants(componantsIn)
{
}

Parallel::Parallel(const Parallel& in)
{
	operator=(in);
}

void Parallel::operator=(const Parallel& in)
{
	componants.clear();
	componants.reserve(in.componants.size());
	for(Componant* componant : in.componants)
		componants.push_back(copy(componant));
}

Parallel::~Parallel()
{
	for(Componant* componant : componants)
		delete componant;
}

std::complex<double> Parallel::execute(double omega)
{
	std::complex<double> accum(0,0);
	for(Componant* componant : componants)
	{
		accum += std::complex<double>(1,0)/componant->execute(omega);
	}
	return std::complex<double>(1,0)/accum;
}

Serial::Serial(std::vector<Componant*> componantsIn): componants(componantsIn)
{
}

Serial::Serial(const Serial& in)
{
	operator=(in);
}

void Serial::operator=(const Serial& in)
{
	componants.clear();
	componants.reserve(in.componants.size());
	for(Componant* componant : in.componants)
		componants.push_back(copy(componant));
}

Serial::~Serial()
{
	for(Componant* componant : componants)
		delete componant;
}

std::complex<double> Serial::execute(double omega)
{
	std::complex<double> accum(0,0);
	for(Componant* componant : componants)
	{
		accum += componant->execute(omega);
	}
	return accum;
}
