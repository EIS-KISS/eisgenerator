#pragma once
#include <complex>
#include <string>
#include <vector>
#include "componant.h"

namespace eis
{

class Warburg: public Componant
{
private:
	fvalue _A = 2e4;
public:
	Warburg(std::string paramStr);
	Warburg(fvalue a = 2e4);
	virtual std::complex<fvalue> execute(fvalue omega) override;
	virtual std::vector<fvalue> getParam() override;
	virtual void setParam(const std::vector<fvalue>& param) override;
	virtual size_t paramCount() override;
	virtual ~Warburg() = default;
};

}
