#include <iostream>
#include <complex>
#include <chrono>

#include "model.h"

double omegafn(size_t i)
{
	return i*10;
}

int main(int argc, char** argv)
{
	std::vector<double> params = {1.0e3, 1.0e-6, 1.0e3, 1.0e-6};
	std::string modelStr("r-(c-r)r-(cr-(rc-r))");

	std::vector<Model::DataPoint> results;

	Model model(modelStr, params);
	auto start = std::chrono::high_resolution_clock::now();
	for(size_t i = 0; i < 50; ++i)
		results.push_back(model.execute(omegafn(i)));
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	for(const Model::DataPoint& res : results)
		std::cout<<"omega: "<<res.omega<<" real = "<<res.im.real()<<" im = "<<res.im.imag()<<'\n';

	std::cout<<"time taken: "<<duration.count()<<" us"<<'\n';
	return 0;
}
