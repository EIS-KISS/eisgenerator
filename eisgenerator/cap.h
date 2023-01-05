#pragma once
#include <complex>
#include <string>
#include <vector>
#include "componant.h"

namespace eis
{

class Cap: public Componant
{
public:
	Cap(std::string paramStr, size_t count = 10);
	Cap(fvalue c = 1e-6);
	virtual std::complex<fvalue> execute(fvalue omega) override;
	virtual size_t paramCount() override;
	virtual char getComponantChar() const override;
	static constexpr char staticGetComponantChar(){return 'c';}
	virtual ~Cap() = default;
};

}
