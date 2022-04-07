#include "resistor.h"

Resistor::Resistor(double r): _R(r)
{}

std::complex<double> Resistor::execute(double omega)
{
	(void)omega;
	return std::complex<double>(_R, 0);
}
