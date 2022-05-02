#include "componant.h"
#include <assert.h>
#include "paralellseriel.h"
#include "resistor.h"
#include "cap.h"
#include "constantphase.h"
#include "warburg.h"
#include "log.h"

using namespace eis;

Componant* Componant::copy(Componant* componant)
{
	switch(getComponantChar(componant))
	{
		case 'r':
			return new Resistor(*dynamic_cast<Resistor*>(componant));
		case 'c':
			return new Cap(*dynamic_cast<Cap*>(componant));
		case 'p':
			return new Cpe(*dynamic_cast<Cpe*>(componant));
		case 'w':
			return new Warburg(*dynamic_cast<Warburg*>(componant));
		case 'l':
			return new Parallel(*dynamic_cast<Parallel*>(componant));
		case 's':
			return new Serial(*dynamic_cast<Serial*>(componant));
		default:
			Log(Log::ERROR)<<"unimplmented type copy for "<<getComponantChar(componant)<<'\n';
			assert(0);
			break;
	}
	return nullptr;
}

char Componant::getComponantChar(Componant* componant)
{
	if(dynamic_cast<Resistor*>(componant))
		return 'r';
	if(dynamic_cast<Cap*>(componant))
		return 'c';
	if(dynamic_cast<Cpe*>(componant))
		return 'p';
	if(dynamic_cast<Warburg*>(componant))
		return 'w';
	if(dynamic_cast<Parallel*>(componant))
		return 'l';
	if(dynamic_cast<Serial*>(componant))
		return 's';

	return 'x';
}
