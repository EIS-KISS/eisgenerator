#include <iostream>
#include <complex>
#include <chrono>

#include "model.h"
#include "log.h"
#include "normalize.h"

void printComponants(eis::Model& model)
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

void runSingle()
{
	eis::Log(eis::Log::INFO)<<__func__;
	std::string modelStr("w{1e3}p{1e-7, 0.7}");

	std::vector<eis::DataPoint> results;

	eis::Model model(modelStr);

	printComponants(model);

	eis::Range omega(0, 1e6, 50);

	auto start = std::chrono::high_resolution_clock::now();
	results = model.executeSweep(omega);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	for(const eis::DataPoint& res : results)
		eis::Log(eis::Log::INFO)<<"omega: "<<res.omega<<" real = "<<res.im.real()<<" im = "<<res.im.imag();

	eis::Log(eis::Log::INFO)<<"time taken: "<<duration.count()<<" us";
}

void sweepCb(std::vector<eis::DataPoint>& data, const std::vector<fvalue>& parameters)
{
	static size_t i = 0;
	++i;
	if((i & 0x3FF) == 0)
		std::cout<<'.'<<std::flush;
}

void runSweep()
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
	model.executeParamSweep(parameters, omega, &sweepCb);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<std::endl;

	eis::Log(eis::Log::INFO)<<"time taken: "<<duration.count()<<" ms";
}

void runSweepByIndex()
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
	std::vector<eis::DataPoint> results = model.executeParamByIndex(parameters, omega, 0);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	printComponants(model);

	for(const eis::DataPoint& res : results)
		eis::Log(eis::Log::INFO)<<"omega: "<<res.omega<<" real = "<<res.im.real()<<" im = "<<res.im.imag();
	eis::Log(eis::Log::INFO)<<"time taken: "<<duration.count()<<" ms";
}

void runRescale()
{
	std::vector<eis::DataPoint> data;
	for(size_t i = 0; i < 10; ++i)
	{
		eis::DataPoint point;
		point.im = std::complex<fvalue>(i,i);
		point.omega = (i+1)*3;
		data.push_back(point);
	}

	std::cout<<"original: ";
	for(const eis::DataPoint& res : data)
		std::cout<<res.omega<<','<<res.im.real()<<','<<res.im.imag()<<'\n';

	data = eis::rescale(data, 5);

	std::cout<<"rescaled: ";
	for(const eis::DataPoint& res : data)
		std::cout<<res.omega<<','<<res.im.real()<<','<<res.im.imag()<<'\n';
}

int main(int argc, char** argv)
{
	eis::Log::headers = true;
	eis::Log::level = eis::Log::INFO;
	runSingle();
	runSweepByIndex();
	runSweep();
	runRescale();
	return 0;
}
