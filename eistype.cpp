#include "eistype.h"
#include <fstream>

#include "strops.h"
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

void eis::Range::print(int level) const
{
	Log(static_cast<Log::Level>(level))<<"Range "<<start<<'-'<<end<<' '<<count<<" steps"<<(log ? " Log" : "");
}

std::vector<Range> eis::Range::rangesFromParamString(const std::string& paramStr, size_t count)
{
	std::vector<std::string> tokens = tokenize(paramStr, ',');

	std::vector<Range> ranges(tokens.size());
	for(size_t i = 0; i < tokens.size(); ++i)
	{
		bool log = false;
		std::string& token = tokens[i];
		std::vector<std::string> subTokens = tokenize(token, '~');

		if(token.back() == 'l' || token.back() == 'L')
		{
			log = true;
			token.pop_back();
		}

		try
		{
			if(subTokens.size() == 1)
			{
				ranges[i] = Range(std::stod(subTokens[0]), std::stod(subTokens[0]), 1, log);
			}
			else
			{
				ranges[i] = Range(std::stod(subTokens[0]), std::stod(subTokens[1]), count, log);
				if(subTokens.size() > 2)
					Log(Log::WARN)<<"invalid parameter string "<<paramStr<<" more that two arguments at range "<<i;
			}

		}
		catch(const std::invalid_argument& ia)
		{
			Log(Log::WARN)<<"invalid parameter string "<<paramStr;
		}
	}
	return ranges;
}
