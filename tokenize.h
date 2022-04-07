#include <string>
#include <vector>
#include <sstream>

std::vector<std::string> tokenize(const std::string& str, const char delim = ' ')
{
	std::stringstream ss(str);
	std::vector<std::string> tokens;

	std::string token;
	while(getline(ss, token, delim))
		tokens.push_back(token);
	return tokens;
}
