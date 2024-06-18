//SPDX-License-Identifier:         LGPL-3.0-or-later
/* * eisgenerator - a shared libary and application to generate EIS spectra
 * Copyright (C) 2022-2024 Carl Philipp Klemm <carl@uvos.xyz>
 *
 * This file is part of eisgenerator.
 *
 * eisgenerator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eisgenerator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with eisgenerator.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <string>
#include <argp.h>
#include "log.h"

#define xstr(s) str(s)
#define str(s) #s

const char *argp_program_version = "eisgenerator-" xstr(VERSION_MAJOR) "." xstr(VERSION_MINOR) "." xstr(VERSION_PATCH);
const char *argp_program_bug_address = "<carl@uvos.xyz>";
static char doc[] = "Application that generates EIS spectra from model strings";
static char args_doc[] = "";

static struct argp_option options[] =
{
  {"verbose",   'v', 0,      0,  "Show debug messages" },
  {"quiet",     'q', 0,      0,  "only output data" },
  {"param",     's', "[COUNT]",      0,  "parameter sweep steps" },
  {"model",      'm', "[STRING]",    0,  "set model string" },
  {"omega",      'o', "[START~END]", 0,  "set omega range" },
  {"extrapolate",  'a', "[START~END]", 0,  "extrapolate a spectra simulated on the omega range to the one given here" },
  {"omegasteps", 'c', "[COUNT]",     0,  "set omega range steps" },
  {"linear",       'l', 0,      0,  "use linear instead of logarithmic steps" },
  {"normalize", 'n', 0,      0,  "normalize values" },
  {"reduce",    'r', 0,      0,  "reduce values to \"interesting\" range" },
  {"hz",        'h', 0,      0,  "freqency values as temporal frequency instead of angular frequency"},
  {"invert",        'i', 0,      0,  "inverts the imaginary axis"},
  {"noise",        'x', "[AMPLITUDE]",      0,  "add noise to output"},
  {"input-type",   't', "[STRING]",      0,  "set input string type, possible values: eis, boukamp, relaxis, madap"},
  {"mode",         'f', "[STRING]",      0,  "mode, possible values: export, code, script, find-range, export-ranges"},
  {"range-distance",   'd', "[DISTANCE]",      0,  "distance from a previous point where a range is considered \"new\""},
  {"parallel",   'p', 0,      0,  "run on multiple threads"},
  {"skip-linear",   'e', 0,      0,  "dont output param sweeps that create linear nyquist plots"},
  {"default-to-range",   'b', 0,      0,  "if a element has no paramters, default to assigning it a range instead of a single value"},
  {"no-compile",   'z', 0,      0,  "dont compile the model into a shared object"},
  {"save",   'y', "[FILENAME]",      0,  "place to save sweeps"},
  { 0 }
};

enum
{
	INPUT_TYPE_EIS,
	INPUT_TYPE_BOUKAMP,
	INPUT_TYPE_RELAXIS,
	INPUT_TYPE_MADAP,
	INPUT_TYPE_UNKOWN
};

enum
{
	MODE_NORMAL,
	MODE_FIND_RANGE,
	MODE_OUTPUT_RANGE_DATAPOINTS,
	MODE_INVALID,
	MODE_CODE,
	MODE_TORCH_SCRIPT
};

struct Config
{
	std::string modelStr = "c{1e-6}r{1e3}-r{1e3}";
	int inputType = INPUT_TYPE_EIS;
	int mode = MODE_NORMAL;
	size_t paramSteps = 10;
	eis::Range omegaRange;
	bool extrapolate = false;
	eis::Range extrapolateRange;
	bool normalize = false;
	bool reduce = false;
	bool hertz = false;
	bool invert = false;
	bool threaded = false;
	bool skipLinear = false;
	bool defaultToRange = false;
	bool noCompile = false;
	double noise = 0;
	double rangeDistance = 0.35;
	std::string saveFileName;

	Config(): omegaRange(1, 1e6, 50, true)
	{}
};

static int parseInputType(const std::string& str)
{
	if(str == "eis")
		return INPUT_TYPE_EIS;
	else if(str == "boukamp")
		return INPUT_TYPE_BOUKAMP;
	else if(str == "relaxis")
		return INPUT_TYPE_RELAXIS;
	else if(str == "madap")
		return INPUT_TYPE_MADAP;
	return INPUT_TYPE_UNKOWN;
}

static int parseMode(const std::string& str)
{
	if(str == "export")
		return INPUT_TYPE_EIS;
	else if(str == "find-range")
		return MODE_FIND_RANGE;
	else if(str == "export-ranges")
		return MODE_OUTPUT_RANGE_DATAPOINTS;
	else if(str == "code")
		return MODE_CODE;
	else if(str == "script")
		return MODE_TORCH_SCRIPT;
	return MODE_INVALID;
}

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
	Config *config = reinterpret_cast<Config*>(state->input);

	switch (key)
	{
	case 'q':
		eis::Log::level = eis::Log::ERROR;
		break;
	case 'v':
		eis::Log::level = eis::Log::DEBUG;
		break;
	case 's':
		config->paramSteps = std::stod(arg);
		break;
	case 'm':
		config->modelStr.assign(arg);
		break;
	case 'l':
		config->omegaRange.log = false;
		break;
	case 'n':
		config->normalize = true;
		break;
	case 'r':
		config->reduce = true;
		break;
	case 'h':
		config->hertz = true;
		break;
	case 'i':
		config->invert = true;
		break;
	case 'c':
		try
		{
			config->omegaRange.count = static_cast<size_t>(std::stoul(std::string(arg)));
		}
		catch(const std::invalid_argument& ia)
		{
			argp_usage(state);
		}
		break;
	case 'o':
	{
		try
		{
			config->omegaRange = eis::Range::fromString(std::string(arg), config->omegaRange.count);
		}
		catch(const std::invalid_argument& ia)
		{
			argp_usage(state);
		}
		break;
	}
	case 'a':
	{
		try
		{
			config->extrapolateRange = eis::Range::fromString(std::string(arg), config->omegaRange.count);
			config->extrapolateRange.log = config->omegaRange.log;
			config->extrapolate = true;
		}
		catch(const std::invalid_argument& ia)
		{
			argp_usage(state);
		}
		break;
	}
	case 'x':
		config->noise = std::stod(std::string(arg));
		break;
	case 't':
		config->inputType = parseInputType(std::string(arg));
		break;
	case 'f':
		config->mode = parseMode(std::string(arg));
		break;
	case 'p':
		config->threaded = true;
		break;
	case 'e':
		config->skipLinear = true;
		break;
	case 'd':
		config->rangeDistance = std::stod(std::string(arg));
		break;
	case 'b':
		config->defaultToRange = true;
		break;
	case 'y':
		config->saveFileName = std::string(arg);
		break;
	case 'z':
		config->noCompile = true;
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

