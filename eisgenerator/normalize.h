#pragma once
#include <vector>
#include <string>

#include "eistype.h"
#include "model.h"

namespace eis
{

void normalize(std::vector<eis::DataPoint>& data);
std::vector<eis::DataPoint> reduceRegion(const std::vector<eis::DataPoint>& data, fvalue gradThreshFactor = 0.01);
std::complex<fvalue> absGrad(const std::vector<eis::DataPoint>& data, size_t index);
void eraseSingularites(std::vector<eis::DataPoint>& data);

}
