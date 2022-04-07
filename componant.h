#pragma once
#include <complex>
#include <iostream>

class Componant
{
	public:
		virtual std::complex<double> execute(double omega)
		{
			std::cout<<"warning incompleat model\n";
			return std::complex<double> (1,0);
		}
};
