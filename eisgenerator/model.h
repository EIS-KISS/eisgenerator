//SPDX-License-Identifier:         LGPL-3.0-or-later
/* * eisgenerator - a shared library and application to generate EIS spectra
 * Copyright (C) 2022-2024 Carl Philipp Klemm <carl@uvos.xyz>
 *
 * This file is part of eisgenerator.
 *
 * eisgenerator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eisgenerator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with eisgenerator.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <complex>
#include <cstddef>
#include <string>
#include <vector>
#include <functional>
#include <kisstype/type.h>

#include "componant/componant.h"

namespace eis
{

struct CompiledObject;

/**
* Eis modeling.
* @defgroup MODELING Modeling
* @{
*/

/**
* @brief The main class of eisgenerator representing an equivalent circuit model.
*/
class Model
{
private:
	Componant *processBrackets(std::string& str, size_t& bracketCounter, size_t paramSweepCount, bool defaultToRange);
	Componant *processBracket(std::string& str, size_t paramSweepCount, bool defaultToRange);
	std::string getParamStr(const std::string& str, size_t index);
	static size_t paramSkipIndex(const std::string& str, size_t index);
	static void addComponantToFlat(Componant* componant, std::vector<Componant*>* flatComponants);

	static void sweepThreadFn(std::vector<std::vector<DataPoint>>* data, Model* model, size_t start, size_t stop, const std::vector<fvalue>& omega);

	size_t getActiveParameterCount();

private:
	Componant *_model = nullptr;
	std::vector<Componant*> _bracketComponants;
	std::string _modelStr;
	std::vector<Componant*> _flatComponants;
	std::string _modelUuid;
	CompiledObject* _compiledModel = nullptr;

public:

	/**
	* @brief Constructor
	*
	* @throws parse_errror If the model string or its parameters include invalid syntax.
	* @param str The model string to create a model object for.
	* @param paramSweepCount Optionally the number of sweep point to use for parameter sweeps.
	* @param defaultToRange Optionally a bool that if true, circuit elements not given in str are defaulted to a range instead of a fixed value.
	*/
	Model(const std::string& str, size_t paramSweepCount = 100, bool defaultToRange = true);
	Model(const Model& in);
	Model& operator=(const Model& in);
	~Model();

	/**
	* @brief Adjusts the sweep count so that the total of spectra that need to be generated in the parameter sweep is close to the given value.
	*
	* @param totalCount The total number of spectra to target.
	* @return The number of spectra that will be generated by a parameter sweep after calling this function, should be close to, but may not be exactly, totalCount.
	*/
	size_t setParamSweepCountClosestTotal(size_t totalCount);

	/**
	* @brief Gets the impedance at the given frequency.
	*
	* This method calls resolveSteps.
	*
	* @param omega The frequency in rad/s to calculate the impedance at.
	* @param index An optional index to the parameter sweep step at which to calculate the impedance.
	* @return A DataPoint struct containing the result.
	*/
	DataPoint execute(fvalue omaga, size_t index = 0);

	/**
	* @brief Executes a frequency sweep along the given range.
	*
	* This method calls resolveSteps.
	*
	* @param omega The range along which to execute a frequency sweep.
	* @param index An optional index to the parameter sweep step at which to calculate the impedance.
	* @return A vector of DataPoint structs containing the impedance at every frequency in the sweep.
	*/
	std::vector<DataPoint> executeSweep(const Range& omega, size_t index = 0);

	/**
	* @brief Executes a frequency sweep with the given omega values.
	*
	* @param omega A vector of frequencies in rad/s to calculate the impedance at.
	* @param index An optional index to the parameter sweep step at which to calculate the impedance.
	* @return A vector of DataPoint structs containing the impedance at every frequency in the sweep.
	*/
	std::vector<DataPoint> executeSweep(const std::vector<fvalue>& omega, size_t index = 0);

	/**
	 * @brief Executes a frequency and parameter sweep at the given parameter indecies
	 *
	 * @param omega The range along which to execute the frequency sweep.
	 * @param indecies the parameter indecies to include in the sweep
	 * @param parallel if this is set to true, the parameter sweep is executed in parallel
	 * @return A vector of vectors of DataPoint structs containing the impedance at every frequency sweep and parameter index.
	 */
	std::vector<std::vector<DataPoint>> executeSweeps(const Range& omega, const std::vector<size_t>& indecies, bool parallel = false);

	/**
	 * @brief Executes a frequency and parameter sweep at the given parameter indecies
	 *
	 * @param omega A vector of frequencies in rad/s to calculate the impedance at.
	 * @param indecies the parameter indecies to include in the sweep
	 * @param parallel if this is set to true, the parameter sweep is executed in parallel
	 * @return A vector of vectors of DataPoint structs containing the impedance at every frequency sweep and parameter index.
	 */
	std::vector<std::vector<DataPoint>> executeSweeps(const std::vector<fvalue>& omega, const std::vector<size_t>& indecies, bool parallel = false);

	/**
	* @brief Executes a frequency sweep with the given omega values for each parameter combination in the applied parameter sweep.
	*
	* This method calls resolveSteps.
	*
	* @param omega The range along which to execute a frequency sweep.
	* @return A vector of vectors of DataPoint structs containing the impedance at every frequency in the sweep with every parameter combination.
	*/
	std::vector<std::vector<DataPoint>> executeAllSweeps(const Range& omega);

	/**
	* @brief Returns the model string corresponding to this model object, without embedded parameters.
	*
	* @return The model string corresponding to this model object, without embedded parameters.
	*/
	std::string getModelStr() const;

	/**
	* @brief Returns the model string corresponding to this model object, with embedded parameters
	*
	* This method calls resolveSteps.
	*
	* @param index The parameter sweep index for which to embed the parameters.
	* @return The model string corresponding to this model object, with embedded parameters.
	*/
	std::string getModelStrWithParam(size_t index);

	/**
	* @brief Returns the model string corresponding to this model object, with embedded parameters.
	*
	* Yhe parameter sweep index of the last call to resolveSteps or 0 is used.
	*
	* @return the model string corresponding to this model object, with embedded parameters
	*/
	std::string getModelStrWithParam() const;

	/**
	* @brief Returns a unique id.
	*
	* This id is only unique for the given circuit description string, not for this object as sutch.
	*
	* @return The uid.
	*/
	size_t getUuid() const;

	/**
	* @brief Returns a vector of pointers to the circuit elements in this model.
	*
	* The pointers can only be assumed to be valid until the next non-const member call to this model object.
	*
	* @param model For internal use only.
	* @return A vector of the circuit elements in the model.
	*/
	std::vector<Componant*> getFlatComponants(Componant *model = nullptr);

	/**
	* @brief Gets the values of the parameters of the circuit elements at the current parameter sweep step.
	*
	* @return The values of the parameters of the circuit elements at the current parameter sweep step.
	*/
	std::vector<fvalue> getFlatParameters();

	/**
	* @brief Gets the ranges of the parameters of the circuit elements.
	*
	* @return The ranges of the parameters of the circuit elements.
	*/
	std::vector<Range> getFlatParameterRanges();

	/**
	* @brief Gets the default ranges of the parameters of each type of circuit element used in the model.
	*
	* @return The default ranges of the parameters of each type of circuit element used in the model.
	*/
	std::vector<Range> getDefaultParameters();

	/**
	* @brief Gets the names of all the parameters in this model
	*
	* @return The names of all the parameters in this model.
	*/
	std::vector<std::string> getParameterNames();

	/**
	* @brief Gets the total number of parameters used by all the circuit elements in this model.
	*
	* @return The total number of parameters.
	*/
	size_t getParameterCount();

	/**
	* @brief This function compiles the model into native vectorized code for faster execution.
	*
	* This function compiles a shared-object for this model that is then loaded by eisgenerator to
	* speed up execution of this model by a factor of 10 or more by employing vectorization and avoiding
	* function calls.
	*
	* This function is slow, but results are cached for the lifetime of process linked to libeisgenerator
	* so that a circuit has to be compiled only once and can then be used by any number of Model objects.
	*
	* This function is only implemented on UNIX, on other platforms this function will always return false.
	* This function also requires that GCC be available in PATH.
	*
	* @return true if compile was successful, false otherwise.
	*/
	bool compile();

	/**
	* @brief This function drops the compiled object code, reverting to graph execution
	*
	*/
	void dropCompiled();

	/**
	* @brief This member determines if the model is in a state ready to execute.
	*
	* @return True if the model is ready for the execute family of methods.
	*/
	bool isReady();

	/**
	* @brief This member resolves the parameters of all circuit elements at the given parameter sweep step.
	*
	* @param index The index to the sweep step to resolve to.
	*/
	void resolveSteps(int64_t index);

	/**
	* @brief Gets the total number of parameter sweep steps for the applied sweep.
	*
	* @return The total number of parameter sweep steps for the applied sweep.
	*/
	size_t getRequiredStepsForSweeps();

	/**
	* @brief Checks if the model is a sweep (i.e. has at least one parameter with a range).
	*
	* @return True if the model contains a parameter sweep, false otherwise.
	*/
	bool isParamSweep();

	/**
	* @brief Compiles this model into efficient c++ code corresponding to the circuit of this model.
	*
	* @return The code or an empty string if compilation was unsuccessfully.
	*/
	std::string getCode();

	/**
	* @brief Compiles this model into TorchScript
	*
	* Can be compiled to GPU kernel bytecode by torch::jit::compile.
	*
	* @return The TorchScript code or an empty string if compilation was unsuccessful.
	*/
	std::string getTorchScript();

	/**
	* @brief Gets the function name in the code for this model.
	*
	* @return The function name in the code for this model.
	*/
	std::string getCompiledFunctionName() const;

	/**
	* @brief Gets a set of indecies that together create a iso-difference set of spectra.
	*
	* Note: this function generates a signifcant fraction of the number of spectra in the models range
	* use getRequiredStepsForSweeps for an estimate of the complexity. It is strongly recommended to call compile()
	* before using this function.
	*
	* @param threaded if this is set to true eisgenerator will spawn nproc number of threads to service this request.
	* @param distance the target distance between subisquent spectra relative to eis::eisDistance.
	* @return A vector of indecies corresponding to the iso-difference spectras.
	*/
	std::vector<size_t> getRecommendedParamIndices(eis::Range omegaRange, double distance, bool threaded = false);

	/**
	* @brief Attempts to check if all elements contribute to the result

	* @param threashold contribution ratio below which the contribution is considered irrelivant
	* @param omegaRange range of frequencies to consider
	* @return True if all Contribute false otherwise
	*/
	bool allElementsContribute(eis::Range omegaRange, fvalue threashold = 0.01);

	/**
	* @brief Checks if all elements in series with one another dont have too similar impedance

	* @param threashold contribution ratio below which the contribution is considered irrelivant
	* @param omegaRange range of frequencies to consider
	* @return True if all series have a difference false otherwise.
	*/
	bool hasSeriesDifference(eis::Range omegaRange, fvalue threashold = 0.1);

	/**
	* @brief Removes the series reistance from a model string (if any)
	*
	* @param model the model string from wich to remove the seires resitance
	*/
	static void removeSeriesResitance(std::string& model);
};

/**
 *  @example simple.cpp
 *  A simple example in how to use eisgenerator.
 */

/**
 *  @example compileing.cpp
 *  An example on how to use the eisgenerator compiled execution model.
 */

/**
 *  @example torchscript.cpp
 *  An example on how to use the eisgenerator torch execution model.
 */

/** @} */

}
