#include <iostream>
#include <complex>
#include <chrono>

#include "model.h"
#include "log.h"
#include "normalize.h"
#include "basicmath.h"

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

static const fvalue mocdata[25][3] = {
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

std::vector<eis::DataPoint> getMockData()
{
	std::vector<eis::DataPoint> data;

	for(size_t i = 0; i < sizeof(mocdata)/sizeof(*mocdata); ++i)
	{
		eis::DataPoint point;
		point.im = std::complex<fvalue>(mocdata[i][1], mocdata[i][2]);
		point.omega = mocdata[i][0];
		data.push_back(point);
	}
	return data;
}

void printComponants(eis::Model& model)
{
	eis::Log(eis::Log::DEBUG)<<"Compnants:";
	for(eis::Componant* componant : model.getFlatComponants())
	{
		eis::Log(eis::Log::DEBUG)<<componant->getComponantChar()<<"{";
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

bool runRescale()
{
	std::cout<<__func__<<'\n';
	std::vector<eis::DataPoint> data;
	for(size_t i = 0; i < 22; ++i)
	{
		eis::DataPoint point;
		point.im = std::complex<fvalue>(i,i);
		point.omega = (i+1)*3;
		data.push_back(point);
	}

	std::cout<<"original:\n";
	printDataVect(data);

	data = eis::rescale(data, 50);

	std::cout<<"rescaled size: "<<data.size()<<" rescale data: \n";
	printDataVect(data);
	return data.size() == 50;
}

void runReduce()
{
	std::cout<<__func__<<'\n';
	std::vector<eis::DataPoint> data = getMockData();

	std::cout<<"original: \n";
	printDataVect(data);

	data = reduceRegion(data);;

	std::cout<<"reduced: \n";
	printDataVect(data);
}

void runNormalize()
{
	std::cout<<__func__<<'\n';
	std::vector<eis::DataPoint> data =getMockData();
	std::cout<<"original"<<'\n';
	printDataVect(data);
	eis::normalize(data);
	std::cout<<"normalized"<<'\n';
	printDataVect(data);

}

void runEraseSingularities()
{
	std::cout<<__func__<<'\n';
	std::vector<eis::DataPoint> data;
	for(size_t i = 0; i < 1000; ++i)
	{
		 eis::DataPoint point;
		 point.im = std::complex<fvalue>(1.0/(i/100.0), 1.0/((i-2)/100.0));
		 point.omega = i;
		 data.push_back(point);
	}

	std::cout<<"original"<<'\n';
	printDataVect(data);

	eraseSingularites(data);

	std::cout<<"erased"<<'\n';
	printDataVect(data);
}

int main(int argc, char** argv)
{
	eis::Log::headers = true;
	eis::Log::level = eis::Log::INFO;
	runSingle();
	runSweepByIndex();
	runSweep();
	runRescale();
	runNormalize();
	runEraseSingularities();
	runReduce();
	return 0;
}
