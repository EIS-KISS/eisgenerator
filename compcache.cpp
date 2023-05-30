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
