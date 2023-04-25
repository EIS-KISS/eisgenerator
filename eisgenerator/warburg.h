#pragma once
#include <complex>
#include <string>
#include <vector>
#include "componant.h"

namespace eis
{

class Warburg: public Componant
{
public:
	Warburg(std::string paramStr, size_t count = 10, bool defaultToRange = false);
	Warburg(fvalue a = 2e4);
	virtual std::complex<fvalue> execute(fvalue omega) override;
	virtual size_t paramCount() override;
	virtual char getComponantChar() const override;
	static constexpr char staticGetComponantChar(){return 'w';}
	virtual std::string componantName() const override {return "Warburg";}
	virtual ~Warburg() = default;
};

}
