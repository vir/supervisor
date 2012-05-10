/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
#ifndef PIDFILE_HPP_INCLUDED
#define PIDFILE_HPP_INCLUDED

class Pidfile
{
	private:
		char * m_path;
	public:
		Pidfile(const char * path);
		~Pidfile();
};

#endif /* PIDFILE_HPP_INCLUDED */

