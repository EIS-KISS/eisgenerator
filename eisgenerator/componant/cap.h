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
	Cap(std::string paramStr, size_t count = 10, bool defaultToRange = false);
	Cap(fvalue c = 1e-6);
	virtual std::complex<fvalue> execute(fvalue omega) override;
	virtual size_t paramCount() const override;
	virtual char getComponantChar() const override;
	static constexpr char staticGetComponantChar(){return 'c';}
	virtual std::string componantName() const override {return "Capacitor";}
	virtual std::string getCode(std::vector<std::string>& parameters) override;
	virtual std::string getTorchScript(std::vector<std::string>& parameters) override;
	virtual std::vector<eis::Range> getDefaultRange(bool range = true) const override;
	virtual ~Cap() = default;
};

}
