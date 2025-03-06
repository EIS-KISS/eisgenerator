#include <chrono>
#include <cassert>
#include <thread>

#include "model.h"
#include "log.h"
#include "compcache.h"


int main()
{
	eis::Log::level = eis::Log::INFO;
	eis::Log(eis::Log::INFO)<<"Eisgenerator Benchmark";

	std::string modelStr = "rp-rp-rp-rl";
	eis::Log::level = eis::Log::ERROR;
	static constexpr long repetitions = 50000;
	eis::Model model(modelStr, 100, true);
	eis::Range omega(0.1, 1e6, 50, true);
	const long datapoints = repetitions*50;
	eis::Log::level = eis::Log::INFO;
	eis::CompCache* cache = eis::CompCache::getInstance();
	std::chrono::high_resolution_clock clock;
	std::vector<size_t> indecies(repetitions);

	eis::Log(eis::Log::INFO)<<"model: "<<modelStr<<" with "<<model.getRequiredStepsForSweeps()<<" steps for sweeps";

	for(size_t i = 0; i < repetitions; ++i)
		indecies[i] = i % model.getRequiredStepsForSweeps();

	cache->dropAllObjects();

	eis::Log(eis::Log::INFO)<<"\nSingle Core:";

	auto start = clock.now();
	for(size_t i = 0; i < repetitions; ++i)
		model.executeSweep(omega, i % model.getRequiredStepsForSweeps());
	auto end = clock.now();
	std::chrono::duration<double> diff = end - start;
	eis::Log(eis::Log::INFO)<<"Graph execution:\t"<<(datapoints*1000000)/std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count()<<" kdp/s";

	model.compile();
	start = clock.now();
	for(size_t i = 0; i < repetitions; ++i)
		model.executeSweep(omega, i % model.getRequiredStepsForSweeps());
	end = clock.now();
	diff = end - start;
	eis::Log(eis::Log::INFO)<<"Compiled execution:\t"<<(datapoints*1000000)/std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count()<<" kdp/s";

	cache->dropAllObjects();
	model.dropCompiled();

	eis::Log(eis::Log::INFO)<<"\nMulti Core with "<<std::thread::hardware_concurrency()<<" threads:";

	start = clock.now();
	model.executeSweeps(omega, indecies, true);
	end = clock.now();
	diff = end - start;
	eis::Log(eis::Log::INFO)<<"Graph execution:\t"<<(datapoints*1000000)/std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count()<<" kdp/s";

	model.compile();
	start = clock.now();
	model.executeSweeps(omega, indecies, true);
	end = clock.now();
	diff = end - start;
	eis::Log(eis::Log::INFO)<<"Compiled execution:\t"<<(datapoints*1000000)/std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count()<<" kdp/s";


}
