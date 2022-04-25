#pragma once
#include <complex>
#include <string>
#include <vector>
#include "componant.h"

class Warburg: public Componant
{
private:
	double _A = 2e4;
public:
	Warburg(std::string paramStr);
	Warburg(double a = 2e4);
	virtual std::complex<double> execute(double omega) override;
	virtual std::vector<double> getParam() override;
	virtual void setParam(const std::vector<double>& param) override;
	virtual size_t paramCount() override;
	virtual ~Warburg() = default;
};
