#pragma once
#include "componant.h"
#include <string>

namespace eis
{

class Resistor: public Componant
{
public:
	Resistor(fvalue r);
	Resistor(std::string paramStr, size_t count = 10, bool defaultToRange = false);
	virtual std::complex<fvalue> execute(fvalue omega)  override;
	virtual size_t paramCount() override;
	virtual char getComponantChar() const override;
	static constexpr char staticGetComponantChar(){return 'r';}
	virtual std::string componantName() const override {return "Resistor";}
	virtual ~Resistor() = default;
};

}
