#pragma once

#include <vector>
#include "eisgenerator/eistype.h"

class LinearRegessionCalculator
{
public:
	std::vector < fvalue > xValues;
	std::vector < fvalue > yValues;

	fvalue slope;
	fvalue offset;
	fvalue stdError;

	LinearRegessionCalculator(const std::vector < fvalue > &xValues, const std::vector < fvalue > &yValues);
};
