#pragma once
#include <complex>
#include <iostream>
#include <vector>

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

		virtual std::vector<fvalue> getParam()
		{
			return std::vector<fvalue>();
		};
		virtual void setParamRanges(const std::vector<eis::Range>& ranges);
		virtual std::vector<eis::Range>& getParamRanges();
		virtual size_t paramCount(){return 0;}
		virtual ~Componant() = default;
		virtual char getComponantChar() const = 0;

		static Componant* copy(Componant* componant);
};

}
