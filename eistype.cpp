//SPDX-License-Identifier:         LGPL-3.0-or-later
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

#include "eistype.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

#include "strops.h"
#include "log.h"
#include "basicmath.h"

using namespace eis;

bool eis::saveToDisk(const EisSpectra& data, const std::filesystem::path& path)
{
	Log(Log::INFO)<<__func__<<" is deprecated";
	return data.saveToDisk(path);
}

EisSpectra eis::loadFromDisk(const std::filesystem::path& path)
{
	Log(Log::INFO)<<__func__<<" is deprecated";
	return EisSpectra(path);
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

eis::Range eis::Range::fromString(std::string str, size_t count)
{
	bool log = false;
	std::vector<std::string> tokens = tokenize(str, '~');
	eis::Range out;

	if(str.back() == 'l' || str.back() == 'L')
	{
		log = true;
		str.pop_back();
	}

	try
	{
		if(tokens.size() == 1)
		{
			out = Range(std::stod(tokens[0]), std::stod(tokens[0]), 1, log);
		}
		else
		{
			out = Range(std::stod(tokens[0]), std::stod(tokens[1]), count, log);
			if(tokens.size() > 2)
				throw std::invalid_argument("");
		}

	}
	catch(const std::invalid_argument& ia)
	{
		throw std::invalid_argument("invalid parameter \""+ str + "\"");
	}

	return out;
}

std::vector<Range> eis::Range::rangesFromParamString(const std::string& paramStr, size_t count)
{
	std::vector<std::string> tokens = tokenize(paramStr, ',');

	std::vector<Range> ranges(tokens.size());
	for(size_t i = 0; i < tokens.size(); ++i)
	{
		std::string& token = tokens[i];
		try
		{
			ranges[i] = fromString(token, count);
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


std::ostream &operator<<(std::ostream &s, Range const& range)
{
	s<<range.getString();
	return s;
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


EisSpectra::EisSpectra(const std::vector<DataPoint>& dataIn, const std::string& modelIn,
	const std::string& headerIn, std::vector<double> labelsIn, std::vector<std::string> labelNamesIn):
data(dataIn), model(modelIn), header(headerIn), labels(labelsIn), labelNames(labelNamesIn)
{

}

EisSpectra::EisSpectra(const std::vector<DataPoint>& dataIn, const std::string& modelIn,
	const std::string& headerIn, std::vector<float> labelsIn, std::vector<std::string> labelNamesIn):
data(dataIn), model(modelIn), header(headerIn), labelNames(labelNamesIn)
{
	setLabels(labelsIn);
}

EisSpectra::EisSpectra(const std::vector<DataPoint>& dataIn, const std::string& modelIn, const std::string& headerIn,
	std::vector<size_t> labelsIn, std::vector<std::string> labelNamesIn):
data(dataIn), model(modelIn), header(headerIn), labelNames(labelNamesIn)
{
	setSzLabels(labelsIn);
}

EisSpectra::EisSpectra(const std::vector<DataPoint>& dataIn, const std::string& modelIn, const std::string& headerIn,
			   size_t label, size_t maxLabel, std::vector<std::string> labelNamesIn):
data(dataIn), model(modelIn), header(headerIn), labelNames(labelNamesIn)
{
	setLabel(label, maxLabel);
}

void EisSpectra::setLabel(size_t label, size_t maxLabel)
{
	labels.assign(maxLabel, 0);
	labels[label] = 1;
}

void EisSpectra::setSzLabels(std::vector<size_t> labelsIn)
{
	labels.assign(labelsIn.size(), 0);
	for(size_t i = 0; i < labelsIn.size(); ++i)
		labels[i] = static_cast<double>(labelsIn[i]);
}

std::vector<size_t> EisSpectra::getSzLabels() const
{
	std::vector<size_t> out(labels.size());
	for(size_t i = 0; i < labels.size(); ++i)
		out[i] = static_cast<size_t>(labels[i]);
	return out;
}

size_t EisSpectra::getLabel()
{
	for(size_t i = 0; i < labels.size(); ++i)
		if(labels[i] != 0)
			return i;
	return 0;
}

bool EisSpectra::isMulticlass()
{
	bool foundFirst = false;
	for(size_t i = 0; i < labels.size(); ++i)
	{
		if(labels[i] != 0)
		{
			if(foundFirst)
				return true;
			else
				foundFirst = true;
		}
	}
	return false;
}

void EisSpectra::setLabels(const std::vector<float>& labelsIn)
{
	labels.assign(labelsIn.size(), 0);
	for(size_t i = 0; i < labels.size(); ++i)
		labels[i] = labelsIn[i];
}

void EisSpectra::setLabels(const std::vector<double>& labelsIn)
{
	labels = labelsIn;
}

std::vector<fvalue> EisSpectra::getFvalueLabels()
{
	if constexpr(std::is_same<fvalue, double>::value)
	{
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wstrict-aliasing"
		return *reinterpret_cast<std::vector<fvalue>*>(&labels);
		#pragma GCC diagnostic pop
	}
	else
	{
		std::vector<fvalue> out(labels.size());
		for(size_t i = 0; i < labels.size(); ++i)
			out[i] = static_cast<fvalue>(labels[i]);
		return out;
	}
}

void EisSpectra::saveToStream(std::ostream& stream) const
{
	stream<<std::scientific;
	stream<<F_MAGIC<<", "<<std::to_string(F_VERSION_MAJOR)<<'.'
		<<std::to_string(F_VERSION_MINOR)<<'.'<<std::to_string(F_VERSION_PATCH)<<'\n';

	stream<<'"'<<model<<'"'<<(!header.empty() ? ", " : "");
	stream<<header;

	if(!labels.empty())
	{
		if(!labelNames.empty())
		{
			stream<<"\nlabelsNames\n";
			std::string labelLine;
			for(const std::string& name : labelNames)
				labelLine += "\"" + name + "\", ";
			labelLine.pop_back();
			labelLine.pop_back();
			stream<<labelLine;
		}
		stream<<"\nlabels\n";

		std::stringstream labelSs;
		for(double label : labels)
			labelSs<<label<<", ";
		std::string labelLine = labelSs.str();
		labelLine.pop_back();
		labelLine.pop_back();
		stream<<labelLine;
	}

	stream<<"\nomega, real, im\n";

	for(const eis::DataPoint& point : data)
		stream<<point.omega<<", "<<point.im.real()<<", "<<point.im.imag()<<'\n';
}

bool EisSpectra::saveToDisk(const std::filesystem::path& path) const
{
	std::fstream file;
	file.open(path, std::ios_base::out | std::ios_base::trunc);
	if(!file.is_open())
	{
		Log(Log::ERROR)<<"can not open "<<path<<" for writing\n";
		return false;
	}

	saveToStream(file);

	file.close();
	return true;
}

EisSpectra EisSpectra::loadFromStream(std::istream& stream)
{
	EisSpectra out;
	std::string line;
	std::getline(stream, line);
	std::vector<std::string> tokens = tokenizeBinaryIgnore(line, ',', '"', '\\');

	if(tokens.size() < 2 || tokens[0] != F_MAGIC)
	{
		throw file_error("not a valid EISGenerator file or stream");
	}
	else
	{
		std::vector<std::string> versionTokens = tokenize(tokens[1], '.');
		if(versionTokens.size() != 3 || std::stoi(versionTokens[0]) > F_VERSION_MAJOR || std::stoi(versionTokens[1]) > F_VERSION_MINOR)
			throw file_error("saved by a newer version of EISGenerator, can not open");
	}

	std::getline(stream, line);
	tokens = tokenizeBinaryIgnore(line, ',', '"', '\\');
	stripQuotes(tokens[0]);
	out.model = tokens[0];
	line.erase(line.begin(), line.begin()+tokens.size());
	out.header = line;

	while(stream.good())
	{
		std::getline(stream, line);
		if(line.starts_with("labelsNames"))
		{
			std::getline(stream, line);
			out.labelNames = tokenizeBinaryIgnore(line, ',', '"', '\\');
			continue;
		}
		else if(line.starts_with("labels"))
		{
			std::getline(stream, line);
			std::vector<std::string> tokens = tokenizeBinaryIgnore(line, ',', '"', '\\');
			for(const std::string& token : tokens)
				out.labels.push_back(std::stod(token));
			continue;
		}
		else if(line.empty() || line[0] == '#' || line.starts_with("omega"))
		{
			continue;
		}
		tokens = tokenize(line, ',');
		if(tokens.size() != 3)
			throw file_error("invalid line: " + line);

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wnarrowing"
		if constexpr (std::is_same<fvalue, double>::value)
			out.data.push_back(DataPoint({std::stod(tokens[1]), std::stod(tokens[2])}, std::stod(tokens[0])));
		else
			out.data.push_back(DataPoint({std::stof(tokens[1]), std::stof(tokens[2])}, std::stof(tokens[0])));
		#pragma GCC diagnostic pop

		eis::removeDuplicates(out.data);
	}

	return out;
}

EisSpectra EisSpectra::loadFromDisk(const std::filesystem::path& path)
{
	std::fstream file;
	file.open(path, std::ios_base::in);
	if(!file.is_open())
		throw file_error("can not open " + path.string() + " for reading\n");

	try
	{
		EisSpectra out = loadFromStream(file);
		return out;
	}
	catch(const file_error& err)
	{
		throw file_error(path.string() + std::string(": ") + err.what());
	}
}

std::ostream &operator<<(std::ostream &s, EisSpectra const& spectra)
{
	spectra.saveToStream(s);
	return s;
}

std::ostream &operator<<(std::ostream &s, DataPoint const& dp)
{
	s<<dp.im;
	return s;
}
