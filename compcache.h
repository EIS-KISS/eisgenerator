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

#include <string>
#include <vector>
#include <map>
#include <complex>
#include <kisstype/type.h>
#include <filesystem>

namespace eis
{

std::string getTempdir();

struct CompiledObject
{
	void* objectCode;
	std::vector<std::complex<fvalue>>(*symbol)(const std::vector<fvalue>&, const std::vector<fvalue>&);
};

class CompCache
{
public:

private:

	inline static CompCache* instance = nullptr;
	std::map<size_t, CompiledObject*> objects;
	CompCache() {};

public:

	static CompCache* getInstance();
	CompCache(const CompCache&) = delete;
	CompCache& operator=(const CompCache&) = delete;
	bool addObject(size_t uuid, const CompiledObject& object);
	CompiledObject* getObject(size_t uuid);
	void dropAllObjects();
};

}
