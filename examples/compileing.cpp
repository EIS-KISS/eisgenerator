#include <iostream>
#include <eisgenerator/model.h>
#include <eisgenerator/log.h>

int main(int argc, char** argv)
{
	// Set the verbosity of libeisgenerator
	eis::Log::level = eis::Log::WARN;

	// Create a model object from a circuit
	eis::Model model("r{100}-r{1000}p{1e-3, 0.5}");

	// Print the corrisponding code for this model
	std::cout<<"C++ code for "<<model.getModelStr()<<":\n"<<model.getCode()<<'\n';

	// Compile the model
	model.compile();

	// Create a model object from a circuit string this time its a parameter sweep
	// This is simply to demonstate that caching is employed
	eis::Model modelSweep("r{10~100}-r{50~1000}p{1e-3, 0.5~0.8}", 50);
	// This now goes quickly
	modelSweep.compile();

	// Create an omega range for a sweep
	// from 1 to 1e4 rad/s, with 100 points
	// and spaced logarithmically
	eis::Range omegaRange(1, 1e4, 10, true);

	// Do a sweep along omegaRange for eatch
	// combination of parameters in the model
	std::vector<std::vector<eis::DataPoint>> data = modelSweep.executeAllSweeps(omegaRange);

	std::cout<<"Got "<<data.size()<<" sweeps with "<<data.size()*data[0].size()<<" total datapoints\n";

	// Print a couple of random ones
	eis::EisSpectra spectraSix(data[6], modelSweep.getModelStrWithParam(6), "Sweep at 6");
	std::cout<<spectraSix<<'\n';

	eis::EisSpectra spectraTen(data[10000], modelSweep.getModelStrWithParam(10000), "Sweep at 10000");
	std::cout<<spectraTen<<'\n';
}
