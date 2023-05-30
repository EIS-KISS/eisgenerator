#include <string>
#include <vector>
#include <map>
#include <complex>

#include "eistype.h"

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
