#include <iostream>
#include <complex>
#include <chrono>

#include "model.h"

double omegafn(size_t i)
{
	return i*10000;
}

int main(int argc, char** argv)
{
	std::string modelStr("r{20e3}p{1e-7, 0.9}");

	std::vector<Model::DataPoint> results;

	Model model(modelStr);
	std::cout<<"Compnants: \n";
	for(Componant* componant : model.getFlatComponants())
	{
		std::cout<<Model::getComponantChar(componant)<<"{";
		for(size_t i = 0; i < componant->paramCount(); ++i)
		{
			std::cout<<componant->getParam()[i];
			if(i != componant->paramCount()-1)
				std::cout<<", ";
		}
		std::cout<<"}\n";
	}

	auto start = std::chrono::high_resolution_clock::now();
	for(size_t i = 0; i < 101; ++i)
		results.push_back(model.execute(omegafn(i)));
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	for(const Model::DataPoint& res : results)
		std::cout<<"omega: "<<res.omega<<" real = "<<res.im.real()<<" im = "<<res.im.imag()<<'\n';

	std::cout<<"time taken: "<<duration.count()<<" us"<<'\n';
	return 0;
}
