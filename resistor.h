#pragma once
#include "componant.h"

class Resistor: public Componant
{
private:
	double _R;

public:
	Resistor(double r);
	virtual std::complex<double> execute(double omega)  override;
};
