/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
#include "childprocess.hpp"
#include "family.hpp"
#include <iostream>
#include <signal.h> // for kill

#if 0
# define XDEBUG(a) a
#else
# define XDEBUG(s)
#endif

int ReadBuffer::do_read()
{
	char buf[8192];
	int r = ::read(m_fd, buf, sizeof(buf));
//	std::cout << "Read " << r << " bytes from fd " << m_fd << std::endl;
	if(r > 0)
		m_buf.append(buf, r);
	return r;
}

bool ReadBuffer::readline(std::string & s)
{
	size_t pos = m_buf.find('\n');
	if(pos != std::string::npos) {
		s = m_buf.substr(0, pos);
		m_buf.erase(0, pos+1);
		return true;
	}
	return false;
}

/***************************/

bool ChildProcess::start()
{
	std::vector<char> cmdbuf(m_family->cmd().begin(), m_family->cmd().end());
	cmdbuf.push_back('\0');
	std::vector<char *> args;
	char * t = strtok(&cmdbuf[0], " \t");
	if(!t)
		exit(-1);
	args.push_back(t);
	while((t = strtok(NULL, " \t"))) { args.push_back(t); }
	XDEBUG(for(int i = 0; i < args.size(); i++) { std::cout << "Arg" << i << ": " << args[i] << std::endl; })
	args.push_back(NULL);

	int fdo[2], fde[2];
	pipe(fdo);
	pipe(fde);
	XDEBUG(std::cout << "Pipes: o: " << fdo[0] << ", " << fdo[1] << ", e: " << fde[0] << ", " << fde[1] << std::endl);
	int pid = fork();
	if(pid < 0) {
		perror("fork");
		return false;
	}
	if(pid) {
		if(m_setpgid)
			setpgid(pid, 0);

		m_pid = pid;
		m_fdo = fdo[0];
		m_fde = fde[0];
		close(fdo[1]);
		close(fde[1]);
		XDEBUG(std::cout << "Closing pipes: " << fdo[1] << ", " << fde[1] << std::endl);
		m_bufo.fd(m_fdo);
		m_bufe.fd(m_fde);
		disp.register_reader(m_fdo, this);
		disp.register_reader(m_fde, this);
		disp.register_chldproc(pid, this);
		return true;
	} else {
		if(m_setpgid)
			setpgid(0, 0);

		signal(SIGINT,  SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
		signal(SIGTTIN, SIG_DFL);
		signal(SIGTTOU, SIG_DFL);
		signal(SIGCHLD, SIG_DFL);

		close(fdo[0]);
		close(fde[0]);
		dup2(fdo[1], 1);
		dup2(fde[1], 2);
		close(fdo[1]);
		close(fde[1]);

		execvp(args[0], &args[0]);
		perror("execvp");
		sleep(1);
		_exit(-1);
	}
	return false;
}

int ChildProcess::can_read(int fd)
{
	int r;
	std::string buf;
	if(fd == m_fdo) {
		r = m_bufo.do_read();
		if(!r) {
			close(m_fdo);
			XDEBUG(std::cout << "Closing pipe: " << m_fdo << std::endl);
			m_fdo = -1;
		}
		while(m_bufo.readline(buf)) {
			m_family->output('O', buf, m_pid);
		}
	} else if(fd == m_fde) {
		r = m_bufe.do_read();
		if(!r) {
			close(m_fde);
			XDEBUG(std::cout << "Closing pipe: " << m_fde << std::endl);
			m_fde = -1;
		}
		while(m_bufe.readline(buf)) {
			m_family->output('E', buf, m_pid);
		}
	} else {
		std::cerr << "ChildProcess::can_read: Not my fd: " << fd << std::endl;
		return -1;
	}
	if(r == 0)
		disp.unregister_reader(fd);
	return r;
}

void ChildProcess::dead(int status)
{
	if(m_fdo >= 0) {
		disp.unregister_reader(m_fdo);
		close(m_fdo);
	}
	if(m_fde >= 0) {
		disp.unregister_reader(m_fde);
		close(m_fde);
	}
	disp.unregister_childproc(m_pid);
	m_family->celebrate_child_death(this, status);
	if(m_setpgid)
		::kill(-m_pid, SIGKILL); // XXX
}

int ChildProcess::kill(int signal)
{
	return ::kill(m_pid, signal);
}


