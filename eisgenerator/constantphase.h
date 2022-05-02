#pragma once
#include <complex>
#include <string>
#include <vector>
#include "componant.h"

namespace eis
{

class Cpe: public Componant
{
private:
	double _Q;
	double _alpha;
public:
	Cpe(std::string paramStr);
	Cpe(double q = 1e-7, double alpha = 0.9);
	virtual std::complex<double> execute(double omega) override;
	virtual std::vector<double> getParam() override;
	virtual void setParam(const std::vector<double>& param) override;
	virtual size_t paramCount() override;
	virtual ~Cpe() = default;
};

}
