#pragma once
#include "componant.h"
#include <string>

namespace eis
{

class Resistor: public Componant
{
private:
	fvalue _R;

public:
	Resistor(fvalue r);
	Resistor(std::string paramStr);
	virtual std::complex<fvalue> execute(fvalue omega)  override;
	virtual std::vector<fvalue> getParam() override;
	virtual void setParam(const std::vector<fvalue>& param) override;
	virtual size_t paramCount() override;
	virtual char getComponantChar() const override;
	static constexpr char staticGetComponantChar(){return 'r';}
	virtual ~Resistor() = default;
};

}
