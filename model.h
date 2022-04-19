#pragma once

#include <complex>
#include <string>
#include <vector>

#include "componant.h"

class Model
{
public:
	struct DataPoint
	{
		std::complex<double> im;
		double omega;
	};
private:
	class Paralell: public Componant
	{
	public:
		std::vector<Componant*> componants;

		Paralell(std::vector<Componant*> componantsIn = std::vector<Componant*>());
		virtual std::complex<double> execute(double omaga) override;
	};

	class Serial: public Componant
	{
	public:
		std::vector<Componant*> componants;

		Serial(std::vector<Componant*> componantsIn = std::vector<Componant*>());
		virtual std::complex<double> execute(double omaga) override;
	};

	size_t opposingBraket(const std::string& str, size_t index, char bracketChar = ')');
	size_t deepestBraket(const std::string& str);
	Componant *processBrackets(std::string& str, size_t& bracketCounter);
	Componant *processBracket(std::string& str);
	std::string getParamStr(const std::string& str, size_t index);
	void addComponantToFlat(Componant* componant);

private:
	Componant *_model = nullptr;
	std::vector<Componant*> _bracketComponants;
	std::string _modelStr;
	std::vector<Componant*> _flatComponants;

public:
	Model(const std::string& str);
	DataPoint execute(double omaga);
	std::string getModelStr();
	std::vector<Componant*> getFlatComponants();
	static char getComponantChar(Componant* componant);
};
