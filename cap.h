#pragma once
#include <complex>
#include "componant.h"

class Cap: public Componant
{
private:
	double _C;
public:
	Cap(double c);
	virtual std::complex<double> execute(double omega) override;
};
