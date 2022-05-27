# eisgenerator

eisgenerator is a shared libary (libeisgenerator.so) and CLI application that allows one to generate [EIS](https://de.wikipedia.org/wiki/Impedanzspektroskopie) spectra with high performance.
The target application is genrating test and example datasets for machine lerning applications.

eisgenerator's development and target platform is UNIX, but it should compile on nigh any platform

## Compile/Install

### Requirements

* git
* c++20 capable compiler (GCC, CLANG)
* cmake 3.20 or later
* optional:
	* gnuplot in $PATH

### Procedure (UNIX)

In a console do:

* git clone https://git-ce.rwth-aachen.de/carl_philipp.klemm/eisgenerator.git
* cd eisgenerator
* mkdir build
* cd build
* cmake ..
* make -j8
* sudo make install

## Basic usage of CLI application

### Generate spectra

eisgenerator_export --model="r{1e3}-r{100}c{1e-6}" --omega=1-1e6 --log --omegasteps=100

flags:

--model: model string, format is simmular to RHD RelaxIS except that the implicit operator is parallel instead of serial, ie. "rc" is a resistor and condenser in parallel while "r-c" is the same in series and that parameters can be passed by adding {number} after the parameter, if an element requires more than one parameter they are specified by adding several comma seperated values in between the brackets like so: {420, 56}

* Supported Elements
	* c: cap
		* one parameter: capacitance {C}
	* r: resistor
		* one parameter: resistance {R}
	* l: inductor
		* one parameter: inductance {L}
	* w: warburg element (infinite), one parameters {A}
		* one parameter: Warburg coefficant {A}
	* p: constant phase element
		* two parameters: {Q, alpha}
	* t: finite approximation of a transmition line
		* three parameters: capacitance, resistance, approximation order {C, R, n}

--omega: range of freqency values (in rad/s) to sweep

--log: use logarithmic steps for omega

--omegasteps: amount of steps to take in the range specified by --omega

further flags can be found with: eisgenerator_export --help

### Plot Spectra

requires [gnuplot](http://www.gnuplot.info/) in $PATH

eisgenerator_plot --model="r{1e3}-r{100}c{1e-6}" --omega=1-1e6 --log --omegasteps=100

Generates Nyquist plot of model specified, allows the usage of the same flags as eisgenerator_export

## Python bindings

python bindings can be build seperately from: [eisgeneratorpy](https://git-ce.rwth-aachen.de/carl_philipp.klemm/eisgeneraorpy)
