#pragma once
#include "componant.h"

namespace eis
{

class FiniteTransmitionline: public Componant
{
	fvalue _C;
	fvalue _R;
	unsigned int _n;

	Componant* subComponant = nullptr;

	static Componant* createTransmitionLine(fvalue c, fvalue r, unsigned int n);
	void updateValues();
	void setDefaultParam(size_t count, bool defaultToRange);

public:
	FiniteTransmitionline(fvalue c, fvalue r, unsigned int n);
	FiniteTransmitionline(std::string paramStr, size_t count = 10, bool defaultToRange = false);
	FiniteTransmitionline(const FiniteTransmitionline& in);
	virtual std::complex<fvalue> execute(fvalue omega) override;
	virtual size_t paramCount() const override;
	virtual std::vector<eis::Range> getDefaultParameters(bool range = true) const override;
	virtual ~FiniteTransmitionline();
	virtual char getComponantChar() const override;
	virtual std::string componantName() const override {return "FiniteTransmitionline";}
	static constexpr char staticGetComponantChar(){return 't';}
};

}
