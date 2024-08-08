#include <fstream>

#include "spectra.h"
#include "eistype.h"
#include "strops.h"
#include "log.h"
#include "basicmath.h"

namespace eis
{

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

	stream<<'"'<<(model.empty() ? "None" : model)<<'"'<<'\n'<<(headerDescription.empty() ? "None" : headerDescription)<<'\n'<<(header.empty() ? "None" : header);

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
	VersionFixed fileVersion;

	if(tokens.size() != 2 || tokens[0] != F_MAGIC)
	{
		throw file_error("not a valid EISGenerator file or stream");
	}
	else
	{
		std::vector<std::string> versionTokens = tokenize(tokens[1], '.');
		if(versionTokens.size() != 3)
			throw file_error("could not load file version from file");
		fileVersion.major = std::stoi(versionTokens[0]);
		fileVersion.minor = std::stoi(versionTokens[1]);
		fileVersion.patch = std::stoi(versionTokens[2]);
		if(fileVersion.major > F_VERSION_MAJOR || fileVersion.minor > F_VERSION_MINOR)
			throw file_error("saved by a newer version of EISGenerator, can not open");
	}

	if(fileVersion.minor == F_VERSION_MINOR)
	{
		std::getline(stream, line);
		stripQuotes(line);
		out.model = line == "None" ? "" : line;
		std::getline(stream, line);
		stripQuotes(line);
		out.headerDescription = line == "None" ? "" : line;
		std::getline(stream, line);
		stripQuotes(line);
		out.header = line == "None" ? "" : line;
	}
	else
	{
		std::getline(stream, line);
		tokens = tokenizeBinaryIgnore(line, ',', '"', '\\');
		stripQuotes(tokens[0]);
		out.model = tokens[0];
		line.erase(line.begin(), line.begin()+tokens.size());
		out.header = line;
	}

	out.model.erase(std::remove(out.model.begin(), out.model.end(), '\0'), out.model.end());
	out.header.erase(std::remove(out.header.begin(), out.header.end(), '\0'), out.header.end());

	while(stream.good())
	{
		std::getline(stream, line);
		if(line.starts_with("labelsNames"))
		{
			std::getline(stream, line);
			out.labelNames = tokenizeBinaryIgnore(line, ',', '"', '\\');
			for(std::string& label : out.labelNames)
			{
				label = stripWhitespace(label);
				stripQuotes(label);
			}
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

}

std::ostream &operator<<(std::ostream &s, eis::EisSpectra const& spectra)
{
	spectra.saveToStream(s);
	return s;
}
