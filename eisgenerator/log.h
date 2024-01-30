//SPDX-License-Identifier:         LGPL-3.0-or-later
/**
* eisgenerator
* Copyright (C) 2021 Carl Klemm
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 3 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the
* Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
*/

#pragma once
#include <iostream>
#include <string>

namespace eis
{

/**
* The logging functions used by eisgenerator
* @defgroup LOG Log
* @{
*/

class Log 
{
public:
	
	enum Level
	{
		DEBUG, /**< Messages useful for debugging */
		INFO,  /**< Messages of intrest to the user incl. progress */
		WARN,  /**< Non fatal errors or problems affecting performance or numeric precision  */
		ERROR  /**< Fatal errors  */
	};

private:
	bool opened = false;
	Level msglevel = DEBUG;
	bool endline = true;

	std::string getLabel(Level level);
	
public:

	static bool headers; /**< If true output will be prefixed with a header detailing the origin of the message */
	static Level level;  /**< Minimum Level required for output to be printed */

	Log() {}
	/**
	* @brief Constructor
	*
	* This Constructor is to be used like a global stream. Thus if logging is desired
	* the user of this api shal use Log(DEBUG)<<"This is a debugging meesage";
	* Using this constructor to create a lvalue has some negative side effects such
	* as: Possibly holding a global lock for the lifetime of the object and potenttaly
	* unintuative newline behavior.
	*
	* @param type The Level to use for this message.
	* @param endline If true an UNIX newline '\n' will be emmited when the oject is destroyed
	*/
	Log(Level type, bool endline = true);
	~Log();
	
	template<class T> Log &operator<<(const T &msg) 
	{
		if(msglevel >= level) 
		{
			std::cout<<msg;
			opened = true;
		}
		return *this;
	}
};

/** @} */

}
