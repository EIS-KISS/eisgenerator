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

#include "compile.h"

#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>
#include <sys/wait.h>

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

		ret = execlp("g++", "gcc", "--shared", "-O2", "-ffast-math", "-ftree-vectorize", "-march=native", "-x", "c++", "-o", outputName.c_str(), "-", NULL);

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

