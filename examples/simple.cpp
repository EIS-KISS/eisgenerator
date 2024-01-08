//
// eisgenerator - a shared libary and application to generate EIS spectra
// Copyright (C) 2022-2024 Carl Philipp Klemm <carl@uvos.xyz>
//
// This file is part of eisgenerator.
//
// eisgenerator is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eisgenerator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with eisgenerator.  If not, see <http://www.gnu.org/licenses/>.
//

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
