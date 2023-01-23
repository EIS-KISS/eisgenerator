#pragma once
#include <string>
#include <exception>

namespace eis
{

//Used by RHD Relaxis3
std::string relaxisToEis(const std::string& in);
std::string eisToRelaxis(const std::string& in);

//Circuit Description Code also known as Boukamp Codeing
std::string cdcToEis(std::string in);
std::string eisToCdc(const std::string& in);

//MADAP string
std::string madapToEis(const std::string& in, const std::string& parameters = "");
std::string eisToMadap(std::string in);

void purgeEisParamBrackets(std::string& in);
}
