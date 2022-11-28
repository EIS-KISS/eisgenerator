#pragma once
#include<string>

//Used by RHD Relaxis3
std::string relaxisToEis(const std::string& in);
std::string eisToRelaxis(const std::string& in);

//Circuit Description Code also known as Boukamp Codeing
std::string cdcToEis(std::string in);
std::string eisToCdc(const std::string& in);

