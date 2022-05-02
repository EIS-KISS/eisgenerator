#pragma once
#include <vector>
#include "model.h"
#include "eistype.h"

namespace eis
{
	std::complex<fvalue> absGrad(const std::vector<eis::DataPoint>& data, size_t index);
	std::complex<fvalue> grad(const std::vector<eis::DataPoint>& data, size_t index);
	std::complex<fvalue> mean(const std::vector<eis::DataPoint>& data);
	std::complex<fvalue> median(const std::vector<eis::DataPoint>& data);
}
