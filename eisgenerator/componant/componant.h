#pragma once
#include <complex>
#include <iostream>
#include <vector>
#include <string>

#include "eistype.h"

namespace eis
{

class Componant
{
	protected:
		std::vector<eis::Range> ranges;
		std::string uniqueName;

	public:
		virtual std::complex<fvalue> execute(fvalue omega)
		{
			(void)omega;
			std::cout<<"warning incompleat model\n";
			return std::complex<fvalue> (1,0);
		}

		virtual void setParamRanges(const std::vector<eis::Range>& ranges);
		virtual std::vector<eis::Range>& getParamRanges();
		virtual std::vector<eis::Range> getParamRanges() const;
		virtual std::vector<eis::Range> getDefaultParameters(bool range = true) const;
		virtual size_t paramCount() const {return 0;};
		virtual ~Componant() = default;
		virtual char getComponantChar() const = 0;
		virtual std::string getComponantString(bool currentValue = true) const;
		virtual std::string componantName() const = 0;
		virtual std::string getCode(std::vector<std::string>& parameters);
		virtual std::string getTorchScript(std::vector<std::string>& parameters);
		virtual bool compileable();

		std::string getUniqueName();

		static Componant* copy(Componant* componant);
		static Componant* createNewComponant(char componant, std::string paramStr = "", size_t count = 1, bool defaultToRange = false);
		static bool isValidComponantChar(char componant);
};

}
