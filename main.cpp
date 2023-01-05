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
		std::cout<<(config.hertz ? res.omega/(2*M_PI) : res.omega)<<','<<res.im.real()<<','<<(config.invert ? 0-res.im.imag() : res.im.imag())<<'\n';

	eis::Log(eis::Log::INFO)<<"time taken: "<<duration.count()<<" us";
}

static void runParamSweep(Config config, eis::Model& model)
{
	eis::Log(eis::Log::INFO)<<"Saving sweep to "<<PARA_SWEEP_OUTPUT_DIR;
	std::filesystem::create_directory(PARA_SWEEP_OUTPUT_DIR);

	size_t count = model.getRequiredStepsForSweeps();
	eis::Log(eis::Log::INFO)<<"Executeing "<<count<<" steps";

	auto start = std::chrono::high_resolution_clock::now();
	for(size_t i = 0; i < count; ++i)
	{
		std::vector<eis::DataPoint> data =  model.executeSweep(config.omegaRange, i);
		size_t outputSize = data.size();
		data = eis::reduceRegion(data);
		data = eis::rescale(data, outputSize);
		eis::saveToDisk(data, std::string(PARA_SWEEP_OUTPUT_DIR)+std::string("/")+std::to_string(++i)+".csv");
		eis::Log(eis::Log::INFO, false)<<'.';
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<std::endl;

	eis::Log(eis::Log::INFO)<<"time taken: "<<duration.count()<<" ms";
}

int main(int argc, char** argv)
{
	eis::Log::level = eis::Log::INFO;
	Config config;
	argp_parse(&argp, argc, argv, 0, 0, &config);

	if(config.hertz)
		config.omegaRange = config.omegaRange*static_cast<fvalue>(2*M_PI);

	if(config.inputType == INPUT_TYPE_BOUKAMP)
		config.modelStr = eis::cdcToEis(config.modelStr);
	else if(config.inputType == INPUT_TYPE_RELAXIS)
		config.modelStr = eis::relaxisToEis(config.modelStr);
	else if(config.inputType == INPUT_TYPE_UNKOWN)
		eis::Log(eis::Log::WARN)<<"Invalid input type specified, assumeing eis";

	eis::Log(eis::Log::INFO)<<"Using model string: "<<config.modelStr;

	eis::Model model(config.modelStr, config.paramSteps);
	printComponants(model);

	if(model.isParamSweep())
		runParamSweep(config, model);
	else
		runSweep(config, model);

	return 0;
}
