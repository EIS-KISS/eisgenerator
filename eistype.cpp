#include "eistype.h"
#include <fstream>
#include <sstream>

#include "strops.h"
#include "log.h"

using namespace eis;

bool eis::saveToDisk(const std::vector<DataPoint>& data, const std::string& fileName, std::string headStr)
{
	std::fstream file;
	file.open(fileName, std::ios_base::out | std::ios_base::trunc);
	if(!file.is_open())
	{
		Log(Log::ERROR)<<"can not open "<<fileName<<" for writing\n";
		return false;
	}

	if(!headStr.empty())
		file<<headStr<<'\n';
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

std::string eis::Range::getString() const
{
	std::stringstream ss;

	ss<<start;
	if(count > 1)
		ss<<'~'<<end;
	if(log)
		ss<<'L';

	return ss.str();
}

bool eis::Range::isSane() const
{
	if(log && (start == 0 || end == 0))
		return false;
	if(end < start)
		return false;
	return true;
}

double eis::eisDistance(const std::vector<eis::DataPoint>& a, const std::vector<eis::DataPoint>& b)
{
	assert(a.size() == b.size());

	double accum = 0;
	for(size_t i = 0; i < a.size(); ++i)
	{
		double diffRe = std::pow(b[i].im.real() - a[i].im.real(), 2);
		double diffIm = std::pow(b[i].im.imag() - a[i].im.imag(), 2);
		accum += std::sqrt(diffRe+diffIm);
	}
	return accum/a.size();
}
