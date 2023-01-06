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

static constexpr double DIST_THRESH = 0.01;
static constexpr char PARA_SWEEP_OUTPUT_DIR[] = "./sweep";

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
		results = eis::reduceRegion(results);
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

	std::cout<<(config.hertz ? "freqency" : "omega")<<",real,im\n";

	for(const eis::DataPoint& res : results)
		std::cout<<(config.hertz ? res.omega/(2*M_PI) : res.omega)<<','<<
			res.im.real()<<','<<(config.invert ? 0-res.im.imag() : res.im.imag())<<'\n';

	eis::Log(eis::Log::INFO)<<"time taken: "<<duration.count()<<" us";
}

static void runParamSweep(const Config& config, eis::Model& model)
{
	eis::Log(eis::Log::INFO)<<"Saving sweep to "<<PARA_SWEEP_OUTPUT_DIR;
	std::filesystem::create_directory(PARA_SWEEP_OUTPUT_DIR);

	size_t count = model.getRequiredStepsForSweeps();
	eis::Log(eis::Log::INFO)<<"Executeing "<<count<<" steps";

	auto start = std::chrono::high_resolution_clock::now();

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
		size_t outputSize = data.size();
		data = eis::reduceRegion(data);
		data = eis::rescale(data, outputSize);
		eis::saveToDisk(data, std::string(PARA_SWEEP_OUTPUT_DIR)+std::string("/")+std::to_string(i)+".csv", model.getModelStrWithParam(i));
		eis::Log(eis::Log::INFO, false)<<'.';
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	eis::Log(eis::Log::INFO, false)<<'\n';

	eis::Log(eis::Log::INFO)<<"time taken: "<<duration.count()<<" ms";
}

void findRanges(const Config& config, eis::Model& model)
{
	std::vector<std::vector<fvalue>> values;
	std::vector<std::vector<eis::DataPoint>> sweeps;
	size_t count = model.getRequiredStepsForSweeps();
	std::vector<eis::Componant*> componants = model.getFlatComponants();
	std::vector<std::vector<eis::DataPoint>> allSweeps;

	if(config.threaded)
		allSweeps = model.executeAllSweeps(config.omegaRange);

	std::vector<char> componantChars;
	for(eis::Componant* componant : componants)
	{
		std::vector<eis::Range>& ranges = componant->getParamRanges();
		for(size_t i = 0; i < ranges.size(); ++i)
			componantChars.push_back(componant->getComponantChar());
	}

	for(size_t i = 0; i < count; ++i)
	{
		std::vector<eis::DataPoint> data;
		if(config.threaded)
			data = allSweeps[i];
		else
			data = model.executeSweep(config.omegaRange, i);
		size_t outputSize = data.size();
		data = eis::reduceRegion(data);
		data = eis::rescale(data, outputSize);

		bool found = false;
		for(ssize_t  i = static_cast<ssize_t>(sweeps.size())-1; i >= 0; --i)
		{
			if(eisDistance(data, sweeps[i]) < DIST_THRESH)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
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

	eis::Log(eis::Log::INFO)<<"Recommended ranges:";
	for(size_t i = 0; i < maxValues.size(); ++i)
	{
		eis::Log(eis::Log::INFO)<<componantChars[i]<<": "<<minValues[i]<<'-'<<maxValues[i];
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

	if(config.inputType == INPUT_TYPE_BOUKAMP)
		config.modelStr = eis::cdcToEis(config.modelStr);
	else if(config.inputType == INPUT_TYPE_RELAXIS)
		config.modelStr = eis::relaxisToEis(config.modelStr);
	else if(config.inputType == INPUT_TYPE_UNKOWN)
		eis::Log(eis::Log::WARN)<<"Invalid input type specified, assumeing eis";

	eis::Log(eis::Log::INFO)<<"Using model string: "<<config.modelStr;

	eis::Model model(config.modelStr, config.paramSteps);
	printComponants(model);

	if(config.findRange && !model.isParamSweep())
	{
		eis::Log(eis::Log::ERROR)<<"Cant find range on a non-sweep model";
		return 1;
	}

	if(model.isParamSweep() && config.findRange)
		findRanges(config, model);
	else if(model.isParamSweep())
		runParamSweep(config, model);
	else
			runSweep(config, model);

	return 0;
}
