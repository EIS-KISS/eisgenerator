#pragma once
#include <complex>
#include <string>
#include <vector>
#include "componant.h"

namespace eis
{

class Cap: public Componant
{
private:
	double _C;
public:
	Cap(std::string paramStr);
	Cap(double c = 1e-6);
	virtual std::complex<double> execute(double omega) override;
	virtual std::vector<double> getParam() override;
	virtual void setParam(const std::vector<double>& param) override;
	virtual size_t paramCount() override;
	virtual ~Cap() = default;
};

}
