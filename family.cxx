/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
#include "defines.h"
#include "family.hpp"
#include "childprocess.hpp"
#include "supervisor.hpp"
#include "logger.hpp"
#include <iostream>
#include <sys/wait.h> // for exit status explanation

#if 0
# define XDEBUG(a) a
#else
# define XDEBUG(s)
#endif

#ifdef USE_SYSLOG
# include <syslog.h>
#endif
#if 0
#include <signal.h> // for sys_siglist (signal "names")
//	extern const char *const sys_siglist[];
inline const char * signal_name(int n)
{
	return sys_siglist[n];
}
#else
//#define _GNU_SOURCE
#include <string.h>
inline const char * signal_name(int n)
{
	return strsignal(n);
}
#endif
static std::string now()
{
	char outstr[200];
	time_t t;
	struct tm *tmp;
	const static char * format = "%Y-%m-%d %H:%M:%S";

	t = time(NULL);
	tmp = localtime(&t);
	if (tmp == NULL) {
		perror("localtime");
		exit(EXIT_FAILURE);
	}

	if (strftime(outstr, sizeof(outstr), format, tmp) == 0) {
		fprintf(stderr, "strftime returned 0");
		exit(EXIT_FAILURE);
	}
	return std::string(outstr);
}


Family::Family(const std::string & name)
	:m_state(ST_RUN), m_name(name)
	, m_autostart(true), m_autorestart(true), m_restart_delay(1)
	, m_logger(0), m_logtailsize(100)
	, m_clones(1)
{
	config.register_context(name, this);
	m_logger = new Logger(m_name);
}

Family::~Family()
{
	if(! m_active.empty()) {
		std::ostringstream ss;
		ss << "WARNING: Family " << m_name << " terminated with " << m_active.size() << " alive children";
		output(ss.str());
	}
	config.unregister_context(m_name);
	delete m_logger;
}

void Family::shutdown(bool initial)
{
	static const int killseq[] = { SIGINT, SIGTERM, SIGTERM, SIGTERM, SIGKILL, SIGKILL, 0 };
	if(m_active.empty())
		return;
	if(initial) {
		m_shutdownphase = 0;
		disp.unregister_alarm(this);
	}
	int sig = killseq[m_shutdownphase];
	if(sig) {
		std::stringstream ss;
		ss << "Sending signal " << sig << " to " << m_active.size() << " children";
		output(ss.str());
		for(std::set<ChildProcess *>::iterator it = m_active.begin(); it != m_active.end(); ++it)
			(*it)->kill(sig);
		m_shutdownphase++;
		disp.register_alarm(2, this);
	} else {
		output("Can not kill children!!! Giving up.");
	}
}

bool Family::start()
{
	if(m_active.size() > m_clones)
		return true;
	unsigned int count = m_clones - m_active.size();
	m_logger->open();
	while(count--) {
		ChildProcess * p = new ChildProcess(this);
		bool started = p->start();
		XDEBUG(std::cerr << "Family::start() active.size = " << m_active.size() << ", count = " << count << ", started: " << (started ? "true" : "false") << std::endl);
		if(started)
			m_active.insert(p);
		else
			delete p;
	}
	return ! m_active.empty();
}

bool Family::stop()
{
	m_state = ST_SHUTDOWN;
	shutdown(true);
	return false;
}

bool Family::restart()
{
	m_state = ST_RESTART;
	shutdown(true);
	return false;
}

bool Family::autostart()
{
	if(m_autostart)
		return start();
	return false;
}

void Family::output(char stream, const std::string & s, pid_t pid, bool forcetimestamp)
{
#ifdef USE_SYSLOG
	if(stream == 'S')
		::syslog(LOG_NOTICE, "(%s) %s", m_name.c_str(), s.c_str());
#endif
	if(m_logger) {
		std::string ll = m_logger->log(std::string(&stream, 1), s, pid, forcetimestamp, (m_clones > 1));
		m_logtail.push_back(ll);
		while(m_logtail.size() > m_logtailsize)
			m_logtail.pop_front();
	} else {
		if(forcetimestamp)
			std::cout << now() << " ";
		std::cout << "OUTPUT(" << m_name << ":" << stream << "): ";
		std::cout << s << std::endl;
	}
}

void Family::celebrate_child_death(ChildProcess * cp, int status)
{
//	ASSERT(cp == m_active);
	m_active.erase(cp);
	std::ostringstream ss;

	ss << "Process " << cp->pid();
	if(WIFEXITED(status)) {
		ss << " exited, status=" << WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
		ss << " killed by signal " << WTERMSIG(status)
			<< " (" << signal_name(WTERMSIG(status)) << ")";
	} else if (WIFSTOPPED(status)) {
		ss << " stopped by signal " << WSTOPSIG(status)
			<< " (" << signal_name(WSTOPSIG(status)) << ")";
	} else if (WIFCONTINUED(status)) {
		ss << "continued";
	}

	output(ss.str(), cp->pid());
	m_active.erase(cp);
	delete cp;
	if((m_state == ST_RUN && m_autorestart) || m_state == ST_RESTART) {
		disp.register_alarm(m_restart_delay, this);
		m_state = ST_RUN;
	}
}

void Family::timeout()
{
	if(m_state == ST_RUN) {
		start();
	} else {
		shutdown(false);
	}
}

void Family::open_log(bool force_reopen)
{
	if(force_reopen)
		m_logger->close();
	m_logger->open();
}

bool Family::configure(const std::string & var, const std::string & value)
{
	//std::cout << " Configure " << m_name << ": " << var << " = " << value << std::endl;
	if(var == "cmd") {
		cmd(value);
		return true;
	} else if(var == "autostart") {
		m_autostart = Config::to_bool(value);
		return true;
	} else if(var == "autorestart") {
		m_autorestart = Config::to_bool(value);
		return true;
	} else if(var == "restartdelay") {
		m_restart_delay = Config::to_int(value);
		return true;
	} else if(var == "clones") {
		int c = Config::to_int(value);
		if(c < 1 || c > 100)
			return false;
		m_clones = c;
		return true;
	}
	return false;
}

ConfTarget * Family::confcontext(const std::string & ctx, bool brackets)
{
	if(ctx == "log") {
		return m_logger;
	}
	return NULL;
}

