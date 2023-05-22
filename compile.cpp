#include "compile.h"

#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>
#include <sys/wait.h>
#include <dlfcn.h>

#include "log.h"

using namespace eis;

static constexpr int PIPE_READ = 0;
static constexpr int PIPE_WRITE = 1;

int eis::compile_code(const std::string& code, const std::string& outputName)
{
	int childStdinPipe[2];
	int childStdoutPipe[2];

	int ret = pipe(childStdinPipe);
	if(ret < 0)
		throw std::runtime_error("Not enough pipe to create child");
	ret = pipe(childStdoutPipe);
	if(ret < 0)
		throw std::runtime_error("Not enough pipe to create child");

	int childPid = fork();

	if(childPid < 0)
		throw std::runtime_error("Unable to create child");

	if(childPid == 0)
	{
		eis::Log(eis::Log::DEBUG)<<"Compile starting";

		if (dup2(childStdinPipe[PIPE_READ], STDIN_FILENO) == -1)
			exit(errno);
		if (dup2(childStdoutPipe[PIPE_WRITE], STDOUT_FILENO) == -1)
			exit(errno);
		if (dup2(childStdoutPipe[PIPE_WRITE], STDERR_FILENO) == -1)
			exit(errno);

		close(childStdinPipe[PIPE_WRITE]);
		close(childStdinPipe[PIPE_READ]);
		close(childStdoutPipe[PIPE_WRITE]);
		close(childStdoutPipe[PIPE_READ]);

		ret = execlp("g++", "gcc", "--shared", "-O2", "-ffast-math", "-ftree-vectorize", "-mavx", "-x", "c++", "-o", outputName.c_str(), "-", NULL);

		exit(ret);
	}
	else
	{
		eis::Log(eis::Log::DEBUG)<<"Sending code to child";

		close(childStdinPipe[PIPE_READ]);
		close(childStdoutPipe[PIPE_WRITE]);

		ret = write(childStdinPipe[PIPE_WRITE], code.c_str(), code.size());
		if(ret < 0)
			throw std::runtime_error("Could not pass code to compiler");
		close(childStdinPipe[PIPE_WRITE]);

		char ch;
		while (read(childStdoutPipe[PIPE_READ], &ch, 1) == 1)
			eis::Log(eis::Log::DEBUG, false)<<ch;

		close(childStdoutPipe[PIPE_READ]);

		eis::Log(eis::Log::DEBUG)<<"Wating for child to exit";

		int exitCode;
		waitpid(childPid, &exitCode, 0);
		if(exitCode != 0)
			eis::Log(eis::Log::ERROR)<<"Failed to compile "<<exitCode;
		eis::Log(eis::Log::DEBUG)<<"Chiled exited";
		return exitCode;
	}
	return -1;
}

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

	return path;
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
