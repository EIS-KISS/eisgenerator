#include "eistype.h"
#include <fstream>

#include "log.h"

using namespace eis;

bool eis::saveToDisk(const std::vector<DataPoint>& data, std::string fileName)
{
	std::fstream file;
	file.open(fileName, std::ios_base::out | std::ios_base::trunc);
	if(!file.is_open())
	{
		Log(Log::ERROR)<<"can not open "<<fileName<<" for writing\n";
		return false;
	}

	file<<"omega,real,im\n";

	for(const eis::DataPoint& point : data)
	{
		file<<point.omega<<','<<point.im.real()<<','<<point.im.imag()<<'\n';
	}
	file.close();
	return true;
}
