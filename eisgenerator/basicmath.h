#pragma once
#include <vector>
#include "model.h"
#include "eistype.h"

namespace eis
{
	std::complex<fvalue> absGrad(const std::vector<eis::DataPoint>& data, size_t index);
	fvalue grad(const std::vector<fvalue>& data, const std::vector<fvalue>& omega, size_t index);
	std::complex<fvalue> grad(const std::vector<eis::DataPoint>& data, size_t index);
	fvalue mean(const std::vector<fvalue>& data);
	std::complex<fvalue> mean(const std::vector<eis::DataPoint>& data);
	fvalue median(std::vector<fvalue> data);
	std::complex<fvalue> median(const std::vector<eis::DataPoint>& data);
	std::vector<eis::DataPoint> rescale(const std::vector<eis::DataPoint>& data, size_t outputSize);
	fvalue pearsonCorrelation(const std::vector<eis::DataPoint>& data);
	fvalue nyquistAreaVariance(const std::vector<eis::DataPoint>& data, eis::DataPoint* centroid = nullptr);
	fvalue maximumNyquistJump(const std::vector<eis::DataPoint>& data);
	void noise(std::vector<eis::DataPoint>& data, double amplitude, bool relative);
	bool fvalueEq(fvalue a, fvalue b, fvalue epsilon = 0.001);
}

