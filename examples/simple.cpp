//SPDX-License-Identifier:        MIT

#include <iostream>
#include <eisgenerator/model.h>
#include <eisgenerator/log.h>

int main(int argc, char** argv)
{
	// Set the verbosity of libeisgenerator
	eis::Log::level = eis::Log::WARN;

	// Create a model object from a circuit string
	eis::Model model("r{100}-r{50}p{1e-3, 0.8}");

	// Generate a signle impedance at omega = 10 rad/s
	eis::DataPoint point = model.execute(10);
	std::cout<<"A calculated data point: "<<point<<'\n';

	// Create an omega range for a sweep
	// from 1 to 1e4 rad/s, with 100 points
	// and spaced logarithmically
	eis::Range omegaRange(1, 1e4, 25, true);

	// Print the values in this range
	std::vector<fvalue> omegaVector = omegaRange.getRangeVector();
	std::cout<<"A Range:\n";
	for(fvalue omega : omegaVector)
		std::cout<<omega<<'\n';
	std::cout<<'\n';

	// Get a spectra from the model and the omega range
	std::vector<eis::DataPoint> data = model.executeSweep(omegaRange);

	// Construct a EisSpectra object for output
	eis::EisSpectra spectra(data, model.getModelStrWithParam(), "My first spectra");

	// Print the spectra
	std::cout<<"Spectra:\n"<<spectra<<'\n';

	// Save it to disk
	spectra.saveToDisk("./spectra.csv");
}
