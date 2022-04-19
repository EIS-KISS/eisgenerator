#pragma once
#include "componant.h"
#include <string>

class Resistor: public Componant
{
private:
	double _R;

public:
	Resistor(double r);
	Resistor(std::string paramStr);
	virtual std::complex<double> execute(double omega)  override;
	virtual std::vector<double> getParam() override;
	virtual void setParam(const std::vector<double>& param) override;
};
