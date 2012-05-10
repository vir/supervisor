/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
#ifndef CHILDPROCESS_HPP_INCLUDED
#define CHILDPROCESS_HPP_INCLUDED
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h> // for strerror
#include <stdlib.h> // for exit

#include <string>
#include "eventdispatcher.hpp"

class ReadBuffer
{
	private:
		std::string m_buf;
		int m_fd;
	public:
		int fd() const { return m_fd; }
		bool fd(int fd) {
			m_fd = fd;
			// Set non-blocking 
			long arg;
			if( (arg = fcntl(m_fd, F_GETFL, NULL)) < 0) { 
				fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
				exit(0); 
			} 
			arg |= O_NONBLOCK; 
			if( fcntl(m_fd, F_SETFL, arg) < 0) { 
				fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
				exit(0); 
			} 
			return true;
		}
		int do_read();
		bool readline(std::string & s);
};

class Family;
class ChildProcess:public AsyncReader, public ChildProc
{
	private:
		int m_pid;
		int m_fdo;
		int m_fde;
		Family * m_family;
		ReadBuffer m_bufo;
		ReadBuffer m_bufe;
	public:
		ChildProcess(Family * f):m_family(f) { }
		virtual ~ChildProcess() { }
		int pid() const { return m_pid; }
		bool start();
		virtual int can_read(int fd);
		virtual void dead(int status);
		virtual int kill(int signal);
};



#endif /* CHILDPROCESS_HPP_INCLUDED */

