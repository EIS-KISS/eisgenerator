#include <iostream>
#include <complex>
#include <chrono>

#include "model.h"

void runSingle()
{
	std::string modelStr("w{20e3}p{1e-7, 0.9}");

	std::vector<Model::DataPoint> results;

	Model model(modelStr);
	std::cout<<"Compnants: \n";
	for(Componant* componant : model.getFlatComponants())
	{
		std::cout<<Componant::getComponantChar(componant)<<"{";
		for(size_t i = 0; i < componant->paramCount(); ++i)
		{
			std::cout<<componant->getParam()[i];
			if(i != componant->paramCount()-1)
				std::cout<<", ";
		}
		std::cout<<"}\n";
	}

	Model::Range omega(0, 1e6, 50);

	auto start = std::chrono::high_resolution_clock::now();
	results = model.sweep(omega);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	for(const Model::DataPoint& res : results)
		std::cout<<"omega: "<<res.omega<<" real = "<<res.im.real()<<" im = "<<res.im.imag()<<'\n';

	Model modelCopy(model);
	results = modelCopy.sweep(omega);

	for(const Model::DataPoint& res : results)
		std::cout<<"omega: "<<res.omega<<" real = "<<res.im.real()<<" im = "<<res.im.imag()<<'\n';


	std::cout<<"time taken: "<<duration.count()<<" us"<<'\n';
}

void sweepCb(std::vector<Model::DataPoint>& data, const std::vector<double>& parameters)
{
	static size_t i = 0;
	++i;
	if((i & 0x3FF) == 0)
		std::cout<<'.'<<std::flush;
}

void runSweep()
{
	std::string modelStr("w{20e3}p{1e-7, 0.9}");
	std::vector<Model::DataPoint> results;

	Model model(modelStr);

	std::vector<Model::Range> parameters;
	parameters.push_back(Model::Range(1e3, 50e3, 100));
	parameters.push_back(Model::Range(1e-7, 20e-7, 100));
	parameters.push_back(Model::Range(0.7, 1.2, 100));

	Model::Range omega(0, 1e6, 25);

	auto start = std::chrono::high_resolution_clock::now();
	model.sweepParams(parameters, omega, &sweepCb);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	std::cout<<"\ntime taken: "<<duration.count()<<" ms"<<'\n';
}

int main(int argc, char** argv)
{
	runSingle();
	//runSweep();
	return 0;
}
