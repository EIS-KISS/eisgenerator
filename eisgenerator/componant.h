#pragma once
#include <complex>
#include <iostream>
#include <vector>

namespace eis
{

class Componant
{
	public:
		virtual std::complex<double> execute(double omega)
		{
			std::cout<<"warning incompleat model\n";
			return std::complex<double> (1,0);
		}

		virtual std::vector<double> getParam()
		{
			return std::vector<double>();
		};
		virtual void setParam(const std::vector<double>& param){};
		virtual size_t paramCount(){return 0;}
		virtual ~Componant() = default;

		static Componant* copy(Componant* componant);
		static char getComponantChar(Componant* componant);
};

}
