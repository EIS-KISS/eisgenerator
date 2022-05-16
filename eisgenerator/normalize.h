#pragma once
#include <vector>
#include <string>

#include "eistype.h"
#include "model.h"

namespace eis
{

void normalize(std::vector<eis::DataPoint>& data);
std::vector<eis::DataPoint> reduceRegion(const std::vector<eis::DataPoint>& data, fvalue gradThreshFactor = 0.01);
void eraseSingularites(std::vector<eis::DataPoint>& data);

}
