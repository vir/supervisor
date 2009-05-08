#include "family.hpp"
#include "childprocess.hpp"
#include "supervisor.hpp"
#include "logger.hpp"
#include <iostream>
#include <sys/wait.h> // for exit status explanation
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
	:m_active(0), m_name(name), m_autostart(true), m_autorestart(true), m_restart_delay(1), m_logger(0), m_logtailsize(100)
{
	config.register_context(name, this);
}

Family::~Family()
{
	config.unregister_context(m_name);
	delete m_logger;
}

bool Family::start()
{
	if(m_active)
		return false;
	if(!m_logger)
		m_logger = new Logger(m_name);
	m_active = new ChildProcess(this);
	return m_active->start();
}

bool Family::autostart()
{
	if(m_autostart)
		return start();
	return false;
}

void Family::output(char stream, const std::string & s, bool forcetimestamp)
{
	if(m_logger) {
		std::string ll = m_logger->log(m_name, std::string(&stream, 1), s, forcetimestamp);
//		m_logtail.push_back(ll); XXX XXX XXX
		if(m_logtail.size() > m_logtailsize)
			m_logtail.pop_front();
	} else {
		if(/*m_append_date ||*/ forcetimestamp)
			std::cout << now() << " ";
		std::cout << "OUTPUT(" << m_name << ":" << stream << "): ";
		std::cout << s << std::endl;
	}
}

void Family::celebrate_child_death(ChildProcess * cp, int status)
{
//	ASSERT(cp == m_active);
	m_active = 0;
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

	output('S', ss.str(), true);
	delete cp;
	/* TODO:
	 * log message about death with status explanation
	 * request restart after configured timeout
	 */
	if(m_autorestart)
		disp.register_alarm(m_restart_delay, this);
}

void Family::timeout()
{
	if(m_autorestart)
		start();
}

bool Family::configure(const std::string & var, const std::string & value)
{
	std::cout << " Configure " << m_name << ": " << var << " = " << value << std::endl;
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
	}
	return false;
}

ConfTarget * Family::confcontext(const std::string & ctx, bool brackets)
{
	std::cout << "Family::confcontext(" << ctx << ", " << brackets << ")" << std::endl;
	if(ctx == "log") {
		if(!m_logger)
			m_logger = new Logger(m_name);
		return m_logger;
	}
	return NULL;
}

