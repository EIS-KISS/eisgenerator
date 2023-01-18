#pragma once
#include <complex>
#include <iostream>
#include <vector>
#include <string>

#include "eistype.h"

namespace eis
{

class Componant
{
	protected:
		std::vector<eis::Range> ranges;
	public:
		virtual std::complex<fvalue> execute(fvalue omega)
		{
			(void)omega;
			std::cout<<"warning incompleat model\n";
			return std::complex<fvalue> (1,0);
		}

		virtual void setParamRanges(const std::vector<eis::Range>& ranges);
		virtual std::vector<eis::Range>& getParamRanges();
		virtual std::vector<eis::Range> getParamRanges() const;
		virtual size_t paramCount(){return 0;}
		virtual ~Componant() = default;
		virtual char getComponantChar() const = 0;
		virtual std::string getComponantString(bool currentValue = true) const;

		static Componant* copy(Componant* componant);
};

}
