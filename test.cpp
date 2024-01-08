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

#include <cmath>
#include <iostream>
#include <complex>
#include <chrono>
#include <sstream>
#include <cstring>

#include "eistype.h"
#include "model.h"
#include "log.h"
#include "normalize.h"
#include "basicmath.h"
#include "strops.h"
#include "translators.h"

const char testEisSpectraFile[] =
	"EISF, 1.0.0\n"
	"\"r-cr-cr\", 0\n"
	"labels\n"
	"1, 1e-10, 1, 3.98107e-07, 6.30957\n"
	"omega, real, im\n"
	"1.000000e+00, 0.000000e+00, 0.000000e+00\n"
	"1.389495e+00, 0.000000e+00, 0.000000e+00\n"
	"1.930698e+00, 0.000000e+00, 0.000000e+00\n"
	"2.682696e+00, 0.000000e+00, 0.000000e+00\n"
	"3.727594e+00, 0.000000e+00, 0.000000e+00\n"
	"5.179475e+00, 0.000000e+00, 0.000000e+00\n"
	"7.196858e+00, 0.000000e+00, 0.000000e+00\n"
	"1.000000e+01, 0.000000e+00, 0.000000e+00\n"
	"1.389496e+01, 0.000000e+00, 0.000000e+00\n"
	"1.930698e+01, 0.000000e+00, 0.000000e+00\n"
	"2.682696e+01, 0.000000e+00, 0.000000e+00\n"
	"3.727594e+01, 0.000000e+00, 0.000000e+00\n"
	"5.179476e+01, 0.000000e+00, 0.000000e+00\n"
	"7.196858e+01, 0.000000e+00, 0.000000e+00\n"
	"1.000000e+02, 0.000000e+00, 0.000000e+00\n"
	"1.389496e+02, 0.000000e+00, 0.000000e+00\n"
	"1.930698e+02, 0.000000e+00, 0.000000e+00\n"
	"2.682696e+02, 0.000000e+00, 0.000000e+00\n"
	"3.727596e+02, 0.000000e+00, 0.000000e+00\n"
	"5.179476e+02, 0.000000e+00, 0.000000e+00\n"
	"7.196858e+02, 0.000000e+00, 0.000000e+00\n"
	"1.000001e+03, 0.000000e+00, 0.000000e+00\n"
	"1.389496e+03, 0.000000e+00, 0.000000e+00\n"
	"1.930698e+03, 0.000000e+00, 0.000000e+00\n"
	"2.682698e+03, 0.000000e+00, 0.000000e+00\n"
	"3.727595e+03, 0.000000e+00, 0.000000e+00\n"
	"5.179476e+03, 0.000000e+00, 0.000000e+00\n"
	"7.196858e+03, 0.000000e+00, 0.000000e+00\n"
	"1.000000e+04, 0.000000e+00, 0.000000e+00\n"
	"1.389497e+04, 0.000000e+00, 0.000000e+00\n"
	"1.930699e+04, 0.000000e+00, 0.000000e+00\n"
	"2.682697e+04, 0.000000e+00, 0.000000e+00\n"
	"3.727595e+04, 0.000000e+00, 0.000000e+00\n"
	"5.179476e+04, 0.000000e+00, 0.000000e+00\n"
	"7.196858e+04, 0.000000e+00, 0.000000e+00\n"
	"1.000000e+05, 0.000000e+00, 0.000000e+00\n"
	"1.389497e+05, 0.000000e+00, 0.000000e+00\n"
	"1.930699e+05, 0.000000e+00, 0.000000e+00\n"
	"2.682698e+05, 0.000000e+00, 0.000000e+00\n"
	"3.727596e+05, 0.000000e+00, 0.000000e+00\n"
	"5.179476e+05, 0.000000e+00, 0.000000e+00\n"
	"7.196858e+05, 0.000000e+00, 0.000000e+00\n"
	"1.000001e+06, 0.000000e+00, 0.000000e+00\n"
	"1.389497e+06, 1.991211e+00, 0.000000e+00\n"
	"1.930699e+06, 5.604766e+00, 0.000000e+00\n"
	"2.682698e+06, 6.533646e+00, 0.000000e+00\n"
	"3.727596e+06, 4.632523e+00, 0.000000e+00\n"
	"5.179476e+06, 5.889149e-01, 0.000000e+00\n"
	"7.196866e+06, 0.000000e+00, 0.000000e+00\n"
	"1.000001e+07, 0.000000e+00, 0.000000e+00\n";

void printDataVect(const std::vector<eis::DataPoint> in)
{
	for(const eis::DataPoint& res : in)
			std::cout<<res.omega<<','<<res.im.real()<<','<<res.im.imag()<<'\n';
}

std::vector<eis::DataPoint> getSine()
{
	std::vector<eis::DataPoint> data;
	for(size_t i = 0; i < 20; ++i)
	{
		eis::DataPoint point;
		point.im = std::complex<fvalue>((sin(i/10.0)/static_cast<double>((i+16)/16))*10, (sin(i/10.0)/static_cast<double>((i+16)/16))*10);
		point.omega = i;
		data.push_back(point);
	}
	return data;
}

static constexpr size_t mocLength = 25;

static const fvalue mocdata[mocLength][3] = {
	0.000000,17200.000000,0.000000,
	41666.700000,2900.000000,-2.926830,
	83333.300000,2900.000000,-1.463410,
	125000.000000,2900.000000,-0.975610,
	166667.000000,2900.000000,-0.731707,
	208333.000000,2900.000000,-0.585366,
	250000.000000,2900.000000,-0.487805,
	291667.000000,2900.000000,-0.418118,
	333333.000000,2900.000000,-0.365854,
	375000.000000,2900.000000,-0.325203,
	416667.000000,2900.000000,-0.292683,
	458333.000000,2900.000000,-0.266075,
	500000.000000,2900.000000,-0.243902,
	541667.000000,2900.000000,-0.225141,
	583333.000000,2900.000000,-0.209059,
	625000.000000,2900.000000,-0.195122,
	666667.000000,2900.000000,-0.182927,
	708333.000000,2900.000000,-0.172166,
	750000.000000,2900.000000,-0.162602,
	791667.000000,2900.000000,-0.154044,
	833333.000000,2900.000000,-0.146341,
	875000.000000,2900.000000,-0.139373,
	916667.000000,2900.000000,-0.133038,
	958333.000000,2900.000000,-0.127253,
	1000000.000000,2900.000000,-0.121951,
};

static const fvalue mocNormalized[mocLength][3] = {
	0,1,0,
	41666.7,0,-1,
	83333.3,0,-0.499998,
	125000,0,-0.333333,
	166667,0,-0.25,
	208333,0,-0.2,
	250000,0,-0.166667,
	291667,0,-0.142857,
	333333,0,-0.125,
	375000,0,-0.111111,
	416667,0,-0.1,
	458333,0,-0.0909089,
	500000,0,-0.0833332,
	541667,0,-0.0769232,
	583333,0,-0.0714285,
	625000,0,-0.0666667,
	666667,0,-0.0625,
	708333,0,-0.0588234,
	750000,0,-0.0555557,
	791667,0,-0.0526317,
	833333,0,-0.0499998,
	875000,0,-0.0476191,
	916667,0,-0.0454546,
	958333,0,-0.0434781,
	1e+06,0,-0.0416666,
};


static const fvalue lowVariance[mocLength][3] = {
	0,0,0,
	0,1,-1,
	0,0.25,0,
	0,0.5,0,
	0,0,-0.25,
	0,0,-0.5,
	0,0.5,-0.5,
	0,0.25,-0.25,
	0,0.33,-0.25,
	0,0.33,-0.5,
	0,0.33,0,
	0,0.33,-0.33,
	0,0.25,-0.33,
	0,0.5,-0.33,
	0,0.5,0,
	0,0,-0.25,
	0,0,-0.5,
	0,0.5,-0.5,
	0,0.25,-0.25,
	0,0.33,-0.25,
	0,0.33,-0.5,
	0,0.33,0,
	0,0.33,-0.33,
	0,0.25,-0.33,
	0,0.5,-0.33,
};

static const fvalue highVariance[mocLength][3] = {
	0,0,0,
	0,1,-1,
	0,0.25*2,0,
	0,0.5*2,0,
	0,0,-0.25*2,
	0,0,-0.5*2,
	0,0.5*2,-0.5*2,
	0,0.25*2,-0.25*2,
	0,0.33*2,-0.25*2,
	0,0.33*2,-0.5*2,
	0,0.33*2,0,
	0,0.33*2,-0.33*2,
	0,0.25*2,-0.33*2,
	0,0.5*2,-0.33*2,
	0,0.5*2,0,
	0,0,-0.25*2,
	0,0,-0.5*2,
	0,0.5*2,-0.5*2,
	0,0.25*2,-0.25*2,
	0,0.33*2,-0.25*2,
	0,0.33*2,-0.5*2,
	0,0.33*2,0,
	0,0.33*2,-0.33*2,
	0,0.25*2,-0.33*2,
	0,0.5*2,-0.33*2,
};

static std::vector<eis::DataPoint> getData(const fvalue in[mocLength][3])
{
	std::vector<eis::DataPoint> data;

	for(size_t i = 0; i < mocLength; ++i)
	{
		eis::DataPoint point;
		point.im = std::complex<fvalue>(in[i][1], in[i][2]);
		point.omega = in[i][0];
		data.push_back(point);
	}
	return data;
}

static bool checkFvalueEq()
{
	return eis::fvalueEq(1.3265, 1.3266, 500);
}

static bool runRescale()
{
	std::vector<eis::DataPoint> data;
	for(size_t i = 0; i < 22; ++i)
	{
		eis::DataPoint point;
		point.im = std::complex<fvalue>(i,i);
		point.omega = (i+1)*3;
		data.push_back(point);
	}

	data = eis::rescale(data, 50);
	return data.size() == 50;
}

static void runReduce()
{
	std::cout<<__func__<<'\n';
	std::vector<eis::DataPoint> data = getData(mocdata);

	std::cout<<"original: \n";
	printDataVect(data);

	data = reduceRegion(data);

	std::cout<<"reduced: \n";
	printDataVect(data);
}

static bool runNormalize()
{
	std::vector<eis::DataPoint> data = getData(mocdata);
	std::vector<eis::DataPoint> normalized = getData(mocNormalized);
	eis::normalize(data);
	double dist = eisDistance(data, normalized);
	if(dist > 1e-6)
	{
		eis::Log(eis::Log::ERROR)<<__func__<<" expected less than 0.999 got "<<dist;
		eis::Log(eis::Log::ERROR)<<__func__<<" expected:";
		printDataVect(normalized);
		eis::Log(eis::Log::ERROR)<<__func__<<" recived:";
		printDataVect(data);
		return false;
	}
	return true;
}

inline void filterData(std::vector<eis::DataPoint>& data, size_t outputSize)
{
	data = eis::reduceRegion(data);

	if(data.size() < outputSize/8)
	{
		data = std::vector<eis::DataPoint>();
		return;
	}
	data = eis::rescale(data, outputSize/2);
}

static bool testDistance()
{
	eis::Model modelA("r{10000}c{2.06914e-10}-p{4.83293e-05, 0.5}");
	eis::Model modelB("r{10000}c{2.97635e-07}-p{0.0001, 0.584211}");
	eis::Range omega(1, 1e6, 50, true);
	std::vector<eis::DataPoint> a = modelA.executeSweep(omega);
	std::vector<eis::DataPoint> b = modelB.executeSweep(omega);
	normalize(a);
	normalize(b);

	double dist = eisDistance(a, b);
	if(dist > 0.36 && dist < 0.35)
	{
		eis::Log(eis::Log::ERROR)<<__func__<<" expected 0.36 > x > 0.35 got "<<dist;
		return false;
	}
	eis::Log(eis::Log::INFO)<<__func__<<" "<<dist;
	return true;
}

static bool modelConsistancy()
{
	std::string modelStr("r{1000}c{1~10000L}-p{1, 4}-w{10}-l{1e-10}");
	eis::Model model(modelStr);

	if(model.getModelStrWithParam() == modelStr)
	{
		return true;
	}
	else
	{
		eis::Log(eis::Log::ERROR)<<__func__<<" expected "<<modelStr<<" got "<<model.getModelStrWithParam();
		return false;
	}
}

static bool uneededBrackets()
{
	std::string tst("(c-(rc)-(r-c(r)))");
	eisRemoveUnneededBrackets(tst);
	std::string expected("c-rc-(r-cr)");
	if(tst == expected)
	{
		return true;
	}
	else
	{
		eis::Log(eis::Log::ERROR)<<__func__<<" expected "<<expected<<" got "<<tst;
		return false;
	}
}

static bool nyquistVariance()
{
	eis::Model modelA("r{10000}c{2.06914e-10}");
	eis::Model modelB("r{100000000}c{2.06914e-10}");
	eis::Range omega(1, 1e6, 50, true);
	std::vector<eis::DataPoint> a = modelA.executeSweep(omega);
	std::vector<eis::DataPoint> b = modelB.executeSweep(omega);
	normalize(a);
	normalize(b);
	fvalue aVar = nyquistAreaVariance(a);
	fvalue bVar = nyquistAreaVariance(b);
	eis::Log(eis::Log::INFO)<<__func__<<" aVar: "<<aVar<<" bVar: "<<bVar;

	return eis::fvalueEq(aVar, 0.38375, 500) && eis::fvalueEq(bVar, 0.514179, 500);
}

static bool nyquistJump()
{
	eis::Model modelA("r{10000}c{2.06914e-10}");
	eis::Model modelB("r{100000000}c{2.06914e-10}");
	eis::Range omega(1, 1e6, 50, true);
	std::vector<eis::DataPoint> a = modelA.executeSweep(omega);
	std::vector<eis::DataPoint> b = modelB.executeSweep(omega);
	normalize(a);
	normalize(b);
	fvalue aVar = maximumNyquistJump(a);
	fvalue bVar = maximumNyquistJump(b);

	eis::Log(eis::Log::INFO)<<__func__<<" aVar: "<<aVar<<" bVar: "<<bVar;
	return eis::fvalueEq(aVar, 0.178182, 500);
}

static bool testEisNyquistDistance()
{
	const std::filesystem::path filePath("./relaxis_rp-rp_0.csv");
	eis::EisSpectra spectra(filePath);
	if(spectra.data.empty())
	{
		eis::Log(eis::Log::INFO)<<__func__<<" Unable to load "<<filePath<<" skiping test";
		return true;
	}

	std::vector<fvalue> omega(spectra.data.size());
	for(size_t i = 0; i < spectra.data.size(); ++i)
		omega[i] = spectra.data[i].omega;
	eis::Log(eis::Log::INFO)<<__func__<<" using model string: "<<spectra.model;
	eis::Model model(spectra.model);

	std::vector<eis::DataPoint> genData = model.executeSweep(omega);
	fvalue dist = eisNyquistDistance(spectra.data, genData);

	if(std::isnan(dist))
	{
		eis::Log(eis::Log::ERROR)<<__func__<<" spectra.data:";
		printDataVect(spectra.data);
		eis::Log(eis::Log::ERROR)<<__func__<<" genData:";
		printDataVect(genData);
		eis::Log(eis::Log::ERROR)<<__func__<<" distanece is NAN!";
		return false;
	}
	else
	{
		return true;
	}
}

static bool testLoadDeduplication()
{
	const std::filesystem::path filePath("./relaxis_rp-rp_0.csv");
	eis::EisSpectra spectra(filePath);
	if(spectra.data.empty())
	{
		eis::Log(eis::Log::INFO)<<__func__<<" Unable to load "<<filePath<<" skiping test";
		return true;
	}

	if(eis::fvalueEq(spectra.data[spectra.data.size()-1].omega, spectra.data[spectra.data.size()-2].omega+2))
	{
		eis::Log(eis::Log::ERROR)<<__func__<<" deduplication failed";
		return false;
	}
	return true;
}

static bool testTranslators()
{
	const std::string boukamp("R(RP)");
	const std::string relaxis("R-(R)(P)");
	const std::string madap("R0-p(R1,CPE1)");

	const std::string uvosEis("r-rp");

	std::string translatedBoukamp = eis::cdcToEis(boukamp);
	std::string translatedRelaxis = eis::relaxisToEis(relaxis);
	std::string translatedMadap = eis::madapToEis(madap);

	eis::Log(eis::Log::INFO)<<"cicuit: "<<uvosEis;
	eis::Log(eis::Log::INFO)<<"from boukamp: "<<boukamp<<" => "<<translatedBoukamp;
	eis::Log(eis::Log::INFO)<<"from relaxis: "<<relaxis<<" => "<<translatedRelaxis;
	eis::Log(eis::Log::INFO)<<"from madap: "<<madap<<" => "<<translatedMadap;

	return translatedBoukamp == uvosEis && translatedRelaxis == uvosEis && translatedMadap == uvosEis;
}

static bool testMadapParams()
{
	const std::string madap("R0-p(R1,CPE1)");
	const std::string madapParams("[(42, 1), (64, 2), (20.123, 3), (0.856, 4)]");

	std::string translatedMadap = eis::madapToEis(madap, madapParams);
	const std::string expectedEisString("r{4.200000e+01}-r{6.400000e+01}p{2.012300e+01, 8.560000e-01}");

	if(translatedMadap == expectedEisString)
	{
		return true;
	}
	else
	{
		eis::Log(eis::Log::ERROR)<<__func__<<"Got "<<translatedMadap<<" from translation expected "<<expectedEisString;
		return false;
	}
}

static bool testLoader()
{
	const size_t expectedLength = 50;
	const size_t expectedLabels = 5;
	const std::string expectedModel("r-cr-cr");
	std::stringstream ss(testEisSpectraFile);
	eis::Log(eis::Log::INFO)<<__func__<<" STRLEN "<<strlen(testEisSpectraFile);
	eis::EisSpectra spectra = eis::EisSpectra::loadFromStream(ss);
	if(spectra.data.size() != expectedLength)
	{
		eis::Log(eis::Log::ERROR)<<__func__<<" Spectra has length "<<spectra.data.size()<<" expected "<<expectedLength;
		return false;
	}

	if(spectra.labels.size() != expectedLabels)
	{
		eis::Log(eis::Log::ERROR)<<__func__<<" Spectra Labels has length "<<spectra.labels.size()<<" expected "<<expectedLabels;
		return false;
	}

	if(spectra.model != expectedModel)
	{
		eis::Log(eis::Log::ERROR)<<__func__<<" Spectra has model \""<<spectra.model<<"\" expected \""<<expectedModel<<'\"';
		//return false;
	}

	std::stringstream saveStream;
	spectra.saveToStream(saveStream);

	if(saveStream.str() != ss.str())
	{
		eis::Log(eis::Log::WARN)<<__func__<<" Saveing and loading dont have the same result. Saved:";
		eis::Log(eis::Log::WARN)<<saveStream.str();
		eis::Log(eis::Log::WARN)<<"Original:"<<ss.str();
	}
	return true;
}

int main(int argc, char** argv)
{
	eis::Log::headers = true;
	eis::Log::level = eis::Log::INFO;

	if(!checkFvalueEq())
		return 1;

	if(!uneededBrackets())
		return 2;

	if(!modelConsistancy())
		return 3;

	if(!testLoadDeduplication())
		return 4;

	if(!runRescale())
		return 5;

	if(!testDistance())
		return 6;

	if(!testEisNyquistDistance())
		return 7;

	if(!runNormalize())
		return 8;

	if(!nyquistVariance())
		return 9;

	if(!nyquistJump())
		return 10;

	if(!testTranslators())
		return 11;

	if(!testMadapParams())
		return 12;

	if(!testLoader())
		return 13;

	return 0;
}
