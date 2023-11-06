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
	void setDefaultParam(size_t count, bool defaultToRange);
public:
	Cpe(std::string paramStr, size_t count = 10, bool defaultToRange = false);
	Cpe(fvalue q, fvalue alpha);
	Cpe();
	virtual std::complex<fvalue> execute(fvalue omega) override;
	virtual size_t paramCount() const override;
	virtual char getComponantChar() const override;
	static constexpr char staticGetComponantChar(){return 'p';}
	virtual std::string componantName() const override {return "ConstantPhase";}
	virtual ~Cpe() = default;
	virtual std::vector<eis::Range> getDefaultParameters(bool range = true) const override;
	virtual std::string getCode(std::vector<std::string>& parameters) override;
	virtual std::string getTorchScript(std::vector<std::string>& parameters) override;
};

}
