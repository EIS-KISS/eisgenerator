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
	fvalue _C;
public:
	Cap(std::string paramStr);
	Cap(fvalue c = 1e-6);
	virtual std::complex<fvalue> execute(fvalue omega) override;
	virtual std::vector<fvalue> getParam() override;
	virtual void setParam(const std::vector<fvalue>& param) override;
	virtual size_t paramCount() override;
	virtual ~Cap() = default;
};

}
