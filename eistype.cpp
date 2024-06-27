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
#include <sstream>
#include <string>
#include <vector>

#include "strops.h"
#include "log.h"

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

std::pair<std::valarray<fvalue>, std::valarray<fvalue>> eis::eisToValarrays(const std::vector<eis::DataPoint>& data)
{
	std::valarray<fvalue> real(data.size());
	std::valarray<fvalue> imag(data.size());

	for(size_t i = 0; i < data.size(); ++i)
	{
		real[i] = data[i].im.real();
		imag[i] = data[i].im.imag();
	}

	return {real, imag};
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

static VersionFixed version = {VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH};

const VersionFixed& getVersion()
{
	return version;
}

std::ostream &operator<<(std::ostream &s, DataPoint const& dp)
{
	s<<dp.im;
	return s;
}
