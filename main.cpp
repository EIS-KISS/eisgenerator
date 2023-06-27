#include <iostream>
#include <complex>
#include <chrono>
#include <cmath>
#include <cassert>
#include <filesystem>

#include "basicmath.h"
#include "model.h"
#include "log.h"
#include "options.h"
#include "normalize.h"
#include "translators.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

static constexpr double STEP_THRESH = 0.30;

static void printComponants(eis::Model& model)
{
	eis::Log(eis::Log::DEBUG)<<"Compnants:";
	for(eis::Componant* componant : model.getFlatComponants())
	{
		eis::Log(eis::Log::DEBUG, false)<<componant->getComponantChar()<<"{";
		std::vector<eis::Range>& ranges = componant->getParamRanges();
		assert(componant->paramCount() == ranges.size());

		for(size_t i = 0; i < componant->paramCount(); ++i)
		{
			eis::Log(eis::Log::DEBUG, false)<<ranges[i].getString();
			if(i != componant->paramCount()-1)
				eis::Log(eis::Log::DEBUG, false)<<", ";
		}
		eis::Log(eis::Log::DEBUG)<<"}";
	}
}

static void runSweep(const Config& config, eis::Model& model)
{
	std::vector<eis::DataPoint> results;

	eis::Range omega = config.omegaRange;

	auto start = std::chrono::high_resolution_clock::now();
	results = model.executeSweep(omega);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	if(config.reduce)
	{
		eis::Log(eis::Log::INFO)<<"reduced normalized results:";
		results = eis::reduceRegion(results, 0.01, false);
		results = eis::reduceRegion(results, 0.01, true);
	}
	else if(config.normalize)
	{
		eis::Log(eis::Log::INFO)<<"normalized results:";
		eis::normalize(results);
	}
	else
	{
		eis::Log(eis::Log::INFO)<<"results:";
	}

	if(config.noise > 0)
		eis::noise(results, config.noise, false);

	eis::Log(eis::Log::INFO)<<(config.hertz ? "freqency" : "omega")<<",real,im";

	for(const eis::DataPoint& res : results)
		std::cout<<(config.hertz ? res.omega/(2*M_PI) : res.omega)<<','<<
			res.im.real()<<','<<(config.invert ? 0-res.im.imag() : res.im.imag())<<'\n';

	eis::Log(eis::Log::INFO)<<"time taken: "<<duration.count()<<" us";

	if(!config.saveFileName.empty())
	{
		eis::Log(eis::Log::INFO)<<"Saveing to "<<config.saveFileName;
		eis::EisSpectra(results, model.getModelStrWithParam(0), "").saveToDisk(config.saveFileName);
	}
}

static void runParamSweep(const Config& config, eis::Model& model)
{
	if(config.saveFileName.empty())
	{
		eis::Log(eis::Log::WARN)<<"No save directory provided via --save, sweeps will not be saved to disk!";
	}
	else
	{
		eis::Log(eis::Log::INFO)<<"Saving sweep to "<<config.saveFileName;
		std::filesystem::create_directory(config.saveFileName);
	}

	size_t count = model.getRequiredStepsForSweeps();
	eis::Log(eis::Log::INFO)<<"Executeing "<<count<<" steps";

	if(!config.noCompile)
		model.compile();

	auto start = std::chrono::high_resolution_clock::now();

	std::vector<std::vector<eis::DataPoint>> allSweeps;
	if(config.threaded)
	{
		eis::Log(eis::Log::INFO)<<"Calculateing sweeps in threads";
		allSweeps = model.executeAllSweeps(config.omegaRange);
		eis::Log(eis::Log::INFO)<<"Done";
	}

	for(size_t i = 0; i < count; ++i)
	{
		std::vector<eis::DataPoint> data;
		if(config.threaded)
			data = allSweeps[i];
		else
			data = model.executeSweep(config.omegaRange, i);

		if(!config.saveFileName.empty())
		{
			if(config.normalize)
				eis::normalize(data);
			if(config.reduce)
			{
				size_t initalDataSize = data.size();
				data = eis::reduceRegion(data);
				if(data.size() < initalDataSize/8)
				{
					eis::Log(eis::Log::INFO)<<"\nskipping output for step "<<i
						<<" as data has no interesting region";
					continue;
				}
				//data = eis::rescale(data, initalDataSize);
			}

			if(config.skipLinear && i > 0)
			{
				fvalue correlation = std::abs(pearsonCorrelation(data));
				if(correlation > 0.5)
				{
					eis::Log(eis::Log::INFO)<<"skipping output for step "<<i
						<<" as data is too linear: "<<correlation;
					continue;
				}
			}

			eis::EisSpectra(data, model.getModelStrWithParam(i), "").saveToDisk(config.saveFileName+"/"+std::to_string(i)+".csv");
		}
		eis::Log(eis::Log::INFO, false)<<'.';
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	eis::Log(eis::Log::INFO, false)<<'\n';

	eis::Log(eis::Log::INFO)<<"time taken: "<<duration.count()<<" ms";
}

static std::vector<std::vector<fvalue>> getRangeValuesForModel(const Config& config, eis::Model& model, std::vector<size_t>* indices = nullptr)
{
	std::vector<std::vector<fvalue>> values;
	std::vector<std::vector<eis::DataPoint>> sweeps;
	size_t count = model.getRequiredStepsForSweeps();
	eis::Log(eis::Log::INFO)<<"Executeing "<<count<<" steps";
	std::vector<eis::Componant*> componants = model.getFlatComponants();
	std::vector<std::vector<eis::DataPoint>> allSweeps;

	if(config.threaded)
		allSweeps = model.executeAllSweeps(config.omegaRange);

	for(size_t i = 0; i < count; ++i)
	{
		std::vector<eis::DataPoint> data;
		if(config.threaded)
			data = allSweeps[i];
		else
			data = model.executeSweep(config.omegaRange, i);
		normalize(data);

		fvalue maxJump =  maximumNyquistJump(data);
		if(maxJump > STEP_THRESH)
		{
			eis::Log(eis::Log::DEBUG)<<"skipping output for step "<<i
				<<" is not well centered: "<<maxJump;
			continue;
		}

		fvalue correlation = std::abs(pearsonCorrelation(data));
		if(correlation > 0.8)
		{
			eis::Log(eis::Log::DEBUG)<<"skipping output for step "<<i
				<<" as data is too linear: "<<correlation;
			continue;
		}

		bool found = false;
		for(ssize_t  j = static_cast<ssize_t>(sweeps.size())-1; j >= 0; --j)
		{
			fvalue dist = eisDistance(data, sweeps[j]);
			if(dist < config.rangeDistance)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			if(indices)
				indices->push_back(i);
			sweeps.push_back(data);
			values.push_back(std::vector<fvalue>());
			if(config.threaded)
				model.resolveSteps(i);
			for(eis::Componant* componant : componants)
			{
				std::vector<eis::Range>& ranges = componant->getParamRanges();
				for(const eis::Range& range : ranges)
					values.back().push_back(range.stepValue());
			}
		}
		if(i % 200 == 0)
		{
			eis::Log(eis::Log::INFO, false)<<'.';
			std::cout<<std::flush;
		}
	}
	eis::Log(eis::Log::INFO, false)<<'\n';

	return values;
}

static void findRanges(const Config& config, eis::Model& model)
{
	std::vector<std::vector<fvalue>> values = getRangeValuesForModel(config, model);
	std::vector<eis::Componant*> componants = model.getFlatComponants();

	if(values.empty())
	{
		eis::Log(eis::Log::INFO)<<"Cant recommend a range";
		return;
	}

	std::vector<fvalue> maxValues(values[0].size(), std::numeric_limits<fvalue>::min());
	std::vector<fvalue> minValues(values[0].size(), std::numeric_limits<fvalue>::max());
	for(size_t i = 0; i < values.size(); ++i)
	{
		for(size_t j = 0; j < values[i].size(); ++j)
		{
			if(values[i][j] > maxValues[j])
				maxValues[j] = values[i][j];
			if(values[i][j] < minValues[j])
				minValues[j] = values[i][j];
		}
	}

	size_t i = 0;
	for(eis::Componant* componant : componants)
	{
		std::vector<eis::Range>& ranges = componant->getParamRanges();
		for(eis::Range& range : ranges)
		{
			fvalue mean = (maxValues[i]+minValues[i])/2;
			fvalue var = std::pow(maxValues[i]-minValues[i], 2)/std::pow(mean, 2);
			if(var < 0.1)
			{
				range.start = mean;
				range.end = mean;
				range.count = 1;
			}
			else
			{
				range.start = minValues[i];
				range.end = maxValues[i];
			}
			++i;
		}
	}

	eis::Log(eis::Log::INFO)<<"Recommended ranges:\n"<<model.getModelStrWithParam();
}

static void outputRanges(const Config& config, eis::Model& model)
{
	std::vector<size_t> indices;
	getRangeValuesForModel(config, model, &indices);

	if(indices.empty())
	{
		eis::Log(eis::Log::INFO)<<"Cant recommend a range";
		return;
	}

	for(size_t index : indices)
		std::cout<<model.getModelStrWithParam(index)<<'\n';
}


std::string translateModelString(const std::string& in, int type)
{
	switch(type)
	{
		case INPUT_TYPE_BOUKAMP:
			return eis::cdcToEis(in);
		case INPUT_TYPE_RELAXIS:
			return eis::relaxisToEis(in);
		case INPUT_TYPE_MADAP:
			return eis::madapToEis(in);
		case INPUT_TYPE_EIS:
		default:
			return in;
	}
}

int main(int argc, char** argv)
{
	eis::Log::level = eis::Log::INFO;
	Config config;
	argp_parse(&argp, argc, argv, 0, 0, &config);

	if(config.hertz)
		config.omegaRange = config.omegaRange*static_cast<fvalue>(2*M_PI);

	if(!config.omegaRange.isSane())
	{
		eis::Log(eis::Log::ERROR)<<"The Omega range: "<<config.omegaRange.getString()<<" is invalid";
		return 1;
	}

	if(config.inputType == INPUT_TYPE_UNKOWN)
	{
		eis::Log(eis::Log::ERROR)<<"Invalid input type specified";
		return 1;
	}
	else
	{
		config.modelStr = translateModelString(config.modelStr, config.inputType);
	}

	if(config.inputType != INPUT_TYPE_EIS)
		eis::Log(eis::Log::INFO)<<"Using translated model string: "<<config.modelStr;

	try
	{
		eis::Model model(config.modelStr, config.paramSteps, config.defaultToRange);
		if(!model.isReady())
		{
			eis::Log(eis::Log::ERROR)<<"Unable to parse "
				<<(config.inputType == INPUT_TYPE_EIS ? "provided" : "translated")
				<<" model string";
			return 1;
		}

		eis::Log(eis::Log::INFO)<<"Using parsed model string: "<<model.getModelStrWithParam();

		printComponants(model);

		if(config.mode == MODE_INVALID)
		{
			eis::Log(eis::Log::ERROR)<<"Invalid mode selected";
			return 1;
		}

		if(config.mode == MODE_FIND_RANGE && !model.isParamSweep())
		{
			eis::Log(eis::Log::ERROR)<<"Cant find range on a non-sweep model";
			return 1;
		}

		if(config.mode == MODE_FIND_RANGE)
		{
			findRanges(config, model);
		}
		else if(config.mode == MODE_OUTPUT_RANGE_DATAPOINTS)
		{
			outputRanges(config, model);
		}
		else if(config.mode == MODE_CODE)
		{
			std::cout<<model.getCode();
		}
		else
		{
			if(model.isParamSweep())
				runParamSweep(config, model);
			else
				runSweep(config, model);
		}

	}
	catch(const std::invalid_argument& ia)
	{
		eis::Log(eis::Log::ERROR)<<"Unable to parse model string, "<<ia.what();
		return 1;
	}

	return 0;
}
