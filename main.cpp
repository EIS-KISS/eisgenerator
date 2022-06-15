#include <iostream>
#include <complex>
#include <chrono>
#include <cmath>
#include <filesystem>

#include "basicmath.h"
#include "model.h"
#include "log.h"
#include "options.h"
#include "normalize.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

static constexpr char PARA_SWEEP_OUTPUT_DIR[] = "./sweep";

static void printComponants(eis::Model& model)
{
	eis::Log(eis::Log::DEBUG)<<"Compnants:";
	for(eis::Componant* componant : model.getFlatComponants())
	{
		eis::Log(eis::Log::DEBUG)<<componant->getComponantChar()<<"{";
		for(size_t i = 0; i < componant->paramCount(); ++i)
		{
			eis::Log(eis::Log::DEBUG)<<componant->getParam()[i];
			if(i != componant->paramCount()-1)
				eis::Log(eis::Log::DEBUG)<<", ";
		}
		eis::Log(eis::Log::DEBUG)<<"}";
	}
}

static void runSweep(const std::string& modelString, eis::Range omega, bool normalize = false,
					 bool reduce = false, bool hertz = false, bool invert = false, double noise = 0)
{
	std::vector<eis::DataPoint> results;

	eis::Model model(modelString);

	printComponants(model);

	auto start = std::chrono::high_resolution_clock::now();
	results = model.executeSweep(omega);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	if(reduce)
	{
		eis::Log(eis::Log::INFO)<<"reduced normalized results:";
		results = eis::reduceRegion(results);
	}
	else if(normalize)
	{
		eis::Log(eis::Log::INFO)<<"normalized results:";
		eis::normalize(results);
	}
	else
	{
		eis::Log(eis::Log::INFO)<<"results:";
	}

	if(noise > 0)
		eis::noise(results, noise, false);

	std::cout<<(hertz ? "freqency" : "omega")<<",real,im\n";

	for(const eis::DataPoint& res : results)
		std::cout<<(hertz ? res.omega/(2*M_PI) : res.omega)<<','<<res.im.real()<<','<<(invert ? 0-res.im.imag() : res.im.imag())<<'\n';

	eis::Log(eis::Log::INFO)<<"time taken: "<<duration.count()<<" us";
}

std::vector<eis::Range> rangesFromParamString(const std::string& parameterString, size_t modelParamCount, size_t steps)
{
	std::vector<std::string> tokens = tokenize(parameterString, ',');

	if(tokens.size() != modelParamCount)
	{
		eis::Log(eis::Log::ERROR)<<"Model requires "<<modelParamCount<<" parameters but "<<tokens.size()<<" parameters where provided\n";
		return std::vector<eis::Range>();
	}

	std::vector<eis::Range> ranges;

	try
	{
		for(const std::string& str : tokens)
		{
			std::vector<std::string> subtokens = tokenize(str, '-', '\0', '\0', 'e');
			if(subtokens.size() > 2)
			{
				eis::Log(eis::Log::ERROR)<<"a range requires two numbers only "<<str<<" provides "<<subtokens.size()<<" numbers\n";
				return std::vector<eis::Range>();
			}
			else if(subtokens.size() == 1)
			{
				ranges.push_back(eis::Range(std::stod(subtokens[0]), std::stod(subtokens[0]), 1, true));
			}
			else
			{
				ranges.push_back(eis::Range(std::stod(subtokens[0]), std::stod(subtokens[1]), steps, true));
			}
		}
	}
	catch (const std::invalid_argument& ia)
	{
		eis::Log(eis::Log::ERROR)<<ia.what();
		return std::vector<eis::Range>();
	}
	return ranges;
}

static void paramSweepCb(std::vector<eis::DataPoint>& data, const std::vector<fvalue>& parameters)
{
	static size_t i = 0;
	size_t outputSize = data.size();
	data = eis::reduceRegion(data);
	data = eis::rescale(data, outputSize);
	eis::saveToDisk(data, std::string(PARA_SWEEP_OUTPUT_DIR)+std::string("/")+std::to_string(++i)+".csv");
	eis::Log(eis::Log::INFO, false)<<'.';
}

static void runParamSweep(const std::string& modelstr, const eis::Range& omega, const std::string& parameterString, size_t steps)
{
	std::string modelStr(modelstr);

	eis::Model model(modelStr);

	std::vector<eis::Range> parameters = rangesFromParamString(parameterString, model.getFlatParametersCount(), steps);
	if(parameters.empty())
		return;

	for(const eis::Range& range : parameters)
		range.print(eis::Log::INFO);

	eis::Log(eis::Log::INFO)<<"Saving sweep to "<<PARA_SWEEP_OUTPUT_DIR;
	std::filesystem::create_directory(PARA_SWEEP_OUTPUT_DIR);

	auto start = std::chrono::high_resolution_clock::now();
	model.executeParamSweep(parameters, omega, &paramSweepCb);
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

	switch(config.mode)
	{
		case MODE_SWEEP:
			runSweep(config.modelStr, config.omegaRange, config.normalize, config.reduce, config.hertz, config.invert, config.noise);
			break;
		case MODE_PARAM_SWEEP:
			runParamSweep(config.modelStr, config.omegaRange, config.parameterString, config.paramSteps);
			break;
	}
	return 0;
}
