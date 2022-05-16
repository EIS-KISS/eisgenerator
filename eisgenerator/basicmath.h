#pragma once
#include <vector>
#include "model.h"
#include "eistype.h"

namespace eis
{
	std::complex<fvalue> absGrad(const std::vector<eis::DataPoint>& data, size_t index);
	std::complex<fvalue> grad(const std::vector<eis::DataPoint>& data, size_t index);
	fvalue mean(const std::vector<fvalue>& data);
	std::complex<fvalue> mean(const std::vector<eis::DataPoint>& data);
	fvalue median(std::vector<fvalue> data);
	std::complex<fvalue> median(const std::vector<eis::DataPoint>& data);
	std::vector<eis::DataPoint> rescale(const std::vector<eis::DataPoint>& data, size_t outputSize);
}
