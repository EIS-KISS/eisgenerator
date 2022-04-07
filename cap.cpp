#include "cap.h"

Cap::Cap(double c): _C(c)
{

}

std::complex<double> Cap::execute(double omega)
{
	return std::complex<double>(0, 0.0-(1.0/(_C*omega)));
}
