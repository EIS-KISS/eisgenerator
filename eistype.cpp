#include "eistype.h"
#include <fstream>
#include <sstream>
#include <algorithm>

#include "strops.h"
#include "log.h"

using namespace eis;

bool eis::saveToDisk(const EisSpectra& data, const std::filesystem::path& path)
{
	std::fstream file;
	file.open(path, std::ios_base::out | std::ios_base::trunc);
	if(!file.is_open())
	{
		Log(Log::ERROR)<<"can not open "<<path<<" for writing\n";
		return false;
	}
	file<<std::scientific;

	file<<data.model<<(!data.header.empty() ? ", " : "");
	file<<data.header;
	file<<"\nomega, real, im\n";

	for(const eis::DataPoint& point : data.data)
		file<<point.omega<<", "<<point.im.real()<<", "<<point.im.imag()<<'\n';
	file.close();
	return true;
}

EisSpectra eis::loadFromDisk(const std::filesystem::path& path)
{
	EisSpectra out;
	std::fstream file;
	file.open(path, std::ios_base::in);
	if(!file.is_open())
		throw file_error("can not open " + path.string() + " for reading\n");

	std::string line;
	std::getline(file, line);
	std::vector<std::string> tokens = tokenize(line, ',');
	out.model = tokens[0];
	line.erase(line.begin(), line.begin()+tokens.size());
	out.header = line;
	std::getline(file, line);

	while(file.good())
	{
		std::getline(file, line);
		if(line.empty() || line[0] == '#')
			continue;
		tokens = tokenize(line, ',');
		if(tokens.size() != 3)
			throw file_error("invalid line in " + path.string() + ": " + line);

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wnarrowing"
		if constexpr (std::is_same<fvalue, double>::value)
			out.data.push_back(DataPoint({std::stod(tokens[1]), std::stod(tokens[2])}, std::stod(tokens[0])));
		else
			out.data.push_back(DataPoint({std::stof(tokens[1]), std::stof(tokens[2])}, std::stof(tokens[0])));
		#pragma GCC diagnostic pop
	}

	file.close();
	return out;
}

void eis::Range::print(int level) const
{
	Log(static_cast<Log::Level>(level))<<"Range "<<start<<'-'<<end<<' '<<count<<" steps"<<(log ? " Log" : "");
}

std::vector<fvalue> eis::Range::getRangeVector() const
{
	std::vector<fvalue> out(count, 0);
	for(size_t i = 0; i < count; ++i)
		out[i] = at(i);
	return out;
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
					throw std::invalid_argument("");
			}

		}
		catch(const std::invalid_argument& ia)
		{
			throw std::invalid_argument("invalid parameter string \"{"+ paramStr + "}\"");
		}
	}
	return ranges;
}

std::string eis::Range::getString() const
{
	std::stringstream ss;

	ss<<start;
	if(count > 1)
	{
		ss<<'~'<<end;
		if(log)
			ss<<'L';
	}

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

//Compute simmuliarity on a bode plot
fvalue eis::eisDistance(const std::vector<eis::DataPoint>& a, const std::vector<eis::DataPoint>& b)
{
	assert(a.size() == b.size());

	double accum = 0;
	for(size_t i = 0; i < a.size(); ++i)
	{
		double diffRe = std::pow(b[i].im.real() - a[i].im.real(), 2);
		double diffIm = std::pow(b[i].im.imag() - a[i].im.imag(), 2);
		accum += diffRe+diffIm;
	}
	return sqrt(accum/a.size());
}

//Compute simmuliarity on a nyquist plot
fvalue eis::eisNyquistDistance(const std::vector<eis::DataPoint>& a, const std::vector<eis::DataPoint>& b)
{
	assert(a.size() > 2 && b.size() > 3);
	double accum = 0;
	for(size_t i = 0; i < a.size(); ++i)
	{
		std::vector<std::pair<double, const eis::DataPoint*>> distances;
		for(size_t j = 0; j < b.size(); ++j)
		{
			double diffRe = std::pow(b[j].im.real() - a[i].im.real(), 2);
			double diffIm = std::pow(b[j].im.imag() - a[i].im.imag(), 2);
			std::pair<double, const eis::DataPoint*> dp;
			dp.first = sqrt(diffRe+diffIm);
			dp.second = &b[j];
			distances.push_back(dp);
		}
		std::sort(distances.begin(), distances.end(),
				  [](const std::pair<double, const eis::DataPoint*>& a, const std::pair<double, const eis::DataPoint*>& b) -> bool
				  {return a.first < b.first;});

		eis::DataPoint base = (*distances[0].second)-(*distances[1].second);
		base = base/base.complexVectorLength();
		eis::DataPoint diff = (*distances[0].second)-a[i];
		diff = diff/diff.complexVectorLength();
		fvalue dprod = base.im.real()*diff.im.real() + base.im.imag()*diff.im.imag();
		fvalue dist = diff.complexVectorLength()*(1-dprod);
		accum += std::pow(dist, 2);
	}
	return std::sqrt(accum/a.size());
}
