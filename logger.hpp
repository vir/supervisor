/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
#ifndef LOGGER_HPP_INCLUDED
#define LOGGER_HPP_INCLUDED

#include "config.hpp"
#include <string>
#include <iostream>

class Logger:public ConfTarget
{
	private:
		std::string m_name;
		bool m_append_date;
		bool m_usecs_timestamps;
		std::string m_filename;
		std::ofstream m_file;
	public:
		Logger(const std::string & name);
		virtual ~Logger();
		std::string log(const std::string & source, std::string tag, const std::string msg, bool forcetimestamp = false);
		virtual bool configure(const std::string & var, const std::string & value);
};

#endif /* LOGGER_HPP_INCLUDED */

