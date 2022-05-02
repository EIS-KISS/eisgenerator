#pragma once
#include <vector>
#include <complex>
#include "componant.h"

namespace eis
{

class Parallel: public Componant
{
public:
	std::vector<Componant*> componants;

	Parallel(std::vector<Componant*> componantsIn = std::vector<Componant*>());
	Parallel(const Parallel& in);
	void operator=(const Parallel& in);
	~Parallel();
	virtual std::complex<double> execute(double omaga) override;
};

class Serial: public Componant
{
public:
	std::vector<Componant*> componants;

	Serial(std::vector<Componant*> componantsIn = std::vector<Componant*>());
	Serial(const Serial& in);
	void operator=(const Serial& in);
	~Serial();
	virtual std::complex<double> execute(double omaga) override;
};

}
