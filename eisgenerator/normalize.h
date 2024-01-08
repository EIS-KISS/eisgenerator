#pragma once
#include <vector>
#include <string>

#include "eistype.h"
#include "model.h"


namespace eis
{

/**
Functions for normalization of spectra data
* @defgroup NORM Normalization
* @{
*/

/**
* @brief Normalizes the data to [0,1]
*
* @param data the data to normalize
*/
void normalize(std::vector<eis::DataPoint>& data);

/**
* @brief Reduces the data by removing "uninteresting"  regions
* Uninteresting regions are detemined by takeing the first and second derivative and removeing the areas where these change very little
*
* @param data the data to reduce
* @return the reduced data
*/
std::vector<eis::DataPoint> reduceRegion(const std::vector<eis::DataPoint>& data, fvalue gradThreshFactor = 0.01, bool useSecondDeiv = false);

/**
* @brief Removes INF and NAN from the dataset by interpolateing from neighboring points
*
* This function is not availbe when this libary is compiled with fast math enabled
*
* As a side effect, this function caps spikes to +-INF to +-10 maximally
*
* @param data the data to remove INF and NAN from
*/
void eraseSingularites(std::vector<eis::DataPoint>& data);

/** @} */

}

