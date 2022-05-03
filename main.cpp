#include <iostream>
#include <complex>
#include <chrono>
#include <cmath>

#include "model.h"
#include "log.h"
#include "options.h"
#include "normalize.h"

static void printComponants(eis::Model& model)
{
	eis::Log(eis::Log::DEBUG)<<"Compnants:";
	for(eis::Componant* componant : model.getFlatComponants())
	{
		eis::Log(eis::Log::DEBUG)<<eis::Componant::getComponantChar(componant)<<"{";
		for(size_t i = 0; i < componant->paramCount(); ++i)
		{
			eis::Log(eis::Log::DEBUG)<<componant->getParam()[i];
			if(i != componant->paramCount()-1)
				eis::Log(eis::Log::DEBUG)<<", ";
		}
		eis::Log(eis::Log::DEBUG)<<"}";
	}
}

static void paramSweepCb(std::vector<eis::DataPoint>& data, const std::vector<fvalue>& parameters)
{
	static size_t i = 0;
	++i;
	if((i & 0x3FF) == 0)
		std::cout<<'.'<<std::flush;
}

static void runSweep(const std::string& modelString, eis::Range omega, bool normalize = false, bool reduce = false, bool hertz = false, bool invert = false)
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

	eis::Log(eis::Log::INFO)<<(hertz ? "freqency" : "omega")<<",real,im";

	for(const eis::DataPoint& res : results)
		std::cout<<res.omega/(2*M_PI)<<','<<res.im.real()<<','<<(invert ? 0-res.im.imag() : res.im.imag())<<'\n';

	eis::Log(eis::Log::INFO)<<"time taken: "<<duration.count()<<" us";
}

static void runParamSweep()
{
	eis::Log(eis::Log::INFO)<<__func__;
	std::string modelStr("w{20e3}p{1e-7, 0.9}");

	eis::Model model(modelStr);

	std::vector<eis::Range> parameters;
	parameters.push_back(eis::Range(1e3, 50e3, 100));
	parameters.push_back(eis::Range(1e-7, 20e-7, 100));
	parameters.push_back(eis::Range(0.7, 1.2, 100));

	eis::Range omega(0, 1e6, 25);

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
			runSweep(config.modelStr, config.omegaRange, config.normalize, config.reduce, config.hertz, config.invert);
			break;
		case MODE_PARAM_SWEEP:
			runParamSweep();
			break;
	}
	return 0;
}
