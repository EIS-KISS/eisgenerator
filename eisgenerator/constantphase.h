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
	fvalue _Q;
	fvalue _alpha;
public:
	Cpe(std::string paramStr);
	Cpe(fvalue q = 1e-7, fvalue alpha = 0.9);
	virtual std::complex<fvalue> execute(fvalue omega) override;
	virtual std::vector<fvalue> getParam() override;
	virtual void setParam(const std::vector<fvalue>& param) override;
	virtual size_t paramCount() override;
	virtual char getComponantChar() const override;
	static constexpr char staticGetComponantChar(){return 'p';}
	virtual ~Cpe() = default;

};

}
