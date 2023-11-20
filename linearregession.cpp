#include "linearregession.h"
#include <stdexcept>
#include <cmath>

LinearRegessionCalculator::LinearRegessionCalculator(const std::vector < fvalue > &xValues, const std::vector < fvalue > &yValues)
{
	if(xValues.size() == yValues.size())
	{
		this->xValues = xValues;
		this->yValues = yValues;

		fvalue sumY = 0;
		fvalue sumX = 0;
		fvalue sumXTimesY = 0;
		fvalue sumSquaredX = 0;
		fvalue sumSquaredY = 0;
		for(unsigned i = 0; i < xValues.size(); i++)
		{
			sumY += yValues[i];
			sumX += xValues[i];
			sumSquaredX += xValues[i] * xValues[i];
			sumSquaredY += yValues[i] * yValues[i];
			sumXTimesY += xValues[i] * yValues[i];
		}

		slope = (sumXTimesY - (sumX * sumY) / xValues.size()) / (sumSquaredX - (sumX * sumX) / xValues.size());
		offset = sumY / xValues.size() - slope * (sumX / xValues.size());

		fvalue error =
		    (xValues.size() * sumSquaredY - sumY * sumY -
		     slope * slope * (xValues.size() * sumSquaredX - sumX * sumX)) / (xValues.size() * (xValues.size() - 2));
		stdError = sqrt((error * error * xValues.size()) / (xValues.size() * sumSquaredX - sumX * sumX));
	}
	else
	{
		throw std::invalid_argument("xValues and yValues need to be the same size");
	}
}
