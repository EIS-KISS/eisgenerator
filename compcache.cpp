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

#include "compcache.h"

#include <string>
#include <vector>
#include <map>
#include <complex>
#include <dlfcn.h>

using namespace eis;

std::string eis::getTempdir()
{
	char* tmpEnv = getenv("TMP");
	char* tempEnv = getenv("TEMP");
	char* tempDirEnv = getenv("TEMPDIR");

	std::filesystem::path path;
	if(tmpEnv && std::string(tmpEnv).length() > 1)
		path = tmpEnv;
	else if(tempEnv && std::string(tempEnv).length() > 1)
		path = tempEnv;
	else if(tempDirEnv && std::string(tempDirEnv).length() > 1)
		path = tempDirEnv;
	else
		path = "/tmp";
	path = path/"eis_models";

	if(!std::filesystem::is_directory(path))
	{
		if(!std::filesystem::create_directory(path))
			throw std::runtime_error(path.string() +
				"is not a directory and a directory can not be created at this locaion");
	}

	return path.string();
}


CompCache* CompCache::getInstance()
{
	if(!instance)
		instance = new CompCache();
	return instance;
}

bool CompCache::addObject(size_t uuid, const CompiledObject& object)
{
	CompiledObject* foundobject = getObject(uuid);
	if(foundobject)
		return false;

	objects.insert({uuid, new CompiledObject(object)});
	return true;
}

CompiledObject* CompCache::getObject(size_t uuid)
{
	auto search = objects.find(uuid);
	if(search == objects.end())
		return nullptr;
	else
		return search->second;
}

void CompCache::dropAllObjects()
{
	for(std::pair<size_t, CompiledObject*> object : objects)
	{
		dlclose(object.second->objectCode);
		delete object.second;
	}

	objects.clear();
}
