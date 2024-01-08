#pragma once
#include <string>
#include <exception>
#include <vector>


namespace eis
{

/**
* Eis model string translators to translate between different description strings used by various 3rd parties
* @defgroup TRANS Translators
* @{
*/

/**
* @brief Translates a model string for RHD instruments RelaxIS to the format used by eisgenerator
*
* @throws parse_errror
* @param in the RelaxIS model string
* @param params an optional vector of the parameters of the circuit elements
* @return the eisgenerator model string
*/
std::string relaxisToEis(const std::string& in, const std::vector<double>& params = std::vector<double>());

/**
* @brief Translates a string for this library to the format used by RHD instruments RelaxIS
*
* Note that any parameters embedded into the eisgenerator model string will be striped and
* not included in the RelaxIS model.
*
* @throws parse_errror
* @param in the eisgenerator model string
* @return the RelaxIS model string
*/
std::string eisToRelaxis(const std::string& in);

/**
* @brief Translates a Boukamp Circut Description Code (CDC) string into the format used by this library
*
* @throws parse_errror
* @param in the CDC string
* @return the eisgenerator model string
*/
std::string cdcToEis(std::string in);

/**
* @brief Translates a string for this library a Boukamp Circut Description Code (CDC)
*
* Note that any parameters embedded into the eisgenerator model string will be striped and
* not included in the CDC.
*
* @throws parse_errror
* @param in the eisgenerator model string
* @return the CDC string
*/
std::string eisToCdc(const std::string& in);

/**
* @brief Translates a model description string as used by the MADAP library into the format used by this library

* @param in a model description string as used by the MADAP library
* @param parameters optinally a string containing the parameters of the model in MADAP parameter format
* @return the eisgenerator model string optinally including embedded parameters
*/
std::string madapToEis(const std::string& in, const std::string& parameters = "");

/**
* @brief Translates a string for this library a into the format used by the MADAP library
*
* Note that any parameters embedded into the eisgenerator model string will be striped and
* not included in the MADAP string.
*
* @throws parse_errror
* @param in the eisgenerator model string
* @return the MADAP string
*/
std::string eisToMadap(std::string in);

/**
* @brief Given a eisgenerator model description string, this function will purge all the embedded parameters
*
* @param in an eisgenerator model string
*/
void purgeEisParamBrackets(std::string& in);

/** @} */

}
