/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
#include "defines.h"
#include "supervisor.hpp"

#include <string>
#include <iostream>
#include <map>

#include "family.hpp"
#include "eventdispatcher.hpp"
#include "config.hpp"
#include "pidfile.hpp"

#include "daemonize.h"

#include <unistd.h>
#include <iostream>
#include <string.h>

#ifdef USE_SYSLOG
# include <syslog.h>
#endif

Supervisor super;

Supervisor::Supervisor()
	: m_state(ST_RUN)
	, m_logdir(".")
	, m_background(false)
{
}

Family * Supervisor::create_family(const std::string & name)
{
	Family * f = new Family(name);
	m_fams[f->name()] = f;
	return f;
}

void Supervisor::open_logs(bool reopen)
{
	std::map<std::string, Family *>::iterator it;
	for(it = m_fams.begin(); it != m_fams.end(); it++) {
		it->second->open_log(reopen);
	}
}

void Supervisor::autostart()
{
	std::map<std::string, Family *>::iterator it;
	for(it = m_fams.begin(); it != m_fams.end(); it++) {
		it->second->autostart();
	}
}

void Supervisor::shutdown(bool restart)
{
	if(!background())
		std::cout << "Requested " << (restart?"restart":"shutdown") << std::endl;
#ifdef USE_SYSLOG
	::syslog(LOG_NOTICE, "Requested %s", (restart?"restart":"shutdown"));
#endif
	m_state = restart ? ST_RESTART : ST_SHUTDOWN;
	std::map<std::string, Family *>::iterator it;
	for(it = m_fams.begin(); it != m_fams.end(); it++) {
		it->second->stop();
	}
	disp.register_alarm(1, this); // wait for all children
}

void Supervisor::signal(int signum)
{
	if(signum == SIGHUP)
		shutdown(true);
	else
		shutdown(false);
}

void Supervisor::timeout()
{
	if(count_running()) {
		disp.register_alarm(1, this); // wait for all children
	} else {
		if(!background())
			std::cout << "All dead, good!" << std::endl;
		disp.stop();
	}
}
 
bool Supervisor::configure(const std::string & var, const std::string & value)
{
	//std::cout << "Supervisor::configure(" << var << ", " << value << ")" << std::endl;
	if(var == "logdir") {
		m_logdir = value;
		return true;
	} else if(var == "background") {
		m_background = Config::to_bool(value);
		return true;
	} else if(var == "pidfile") {
		m_pidfile = value;
		return true;
	}
	return false;
}

ConfTarget * Supervisor::confcontext(const std::string & ctx, bool brackets)
{
	std::map<std::string, Family *>::iterator it;
	it = m_fams.find(ctx);
	if(it == m_fams.end() && brackets) {
		return create_family(ctx);
	}
	if(it != m_fams.end())
		return it->second;
	return NULL;
}

unsigned int Supervisor::count_running() const
{
	unsigned int r = 0;
	std::map<std::string, Family *>::const_iterator it;
	for(it = m_fams.begin(); it != m_fams.end(); it++) {
		if(it->second->state() == Family::ST_RUN)
			r++;
	}
	return r;
}

int Supervisor::run()
{
	Pidfile pidfile(m_pidfile.c_str());
	disp.register_signal(SIGHUP, this);
	disp.register_signal(SIGINT, this);
	disp.register_signal(SIGTERM, this);
	autostart();
	return disp.run();
}

void Supervisor::basename(const char * exe)
{
	const char * t = strrchr(exe, '/');
	if(t)
		exe = t + 1;
#ifdef USE_SYSLOG
	::openlog(exe, 0, LOG_DAEMON);
	::syslog(LOG_INFO, "Started");
#endif
#ifdef NAME_HEURISTIC
	size_t len = strcspn(exe, "-_");
	if(exe[len] && exe[len + 1])
		exe += len + 1;
#endif
	m_basename = exe;
}

int main(int argc, char * argv[])
{
	const char * conffile = NULL;
	std::vector<const char *> overrides;
	int daemon = 0;
	int ch;
	while ((ch = getopt(argc, argv, "?hc:Ds:")) != -1) {
		switch (ch) {
			case 'c':
				conffile = optarg;
				break;
			case 'D':
				daemon++;
				break;
			case 's':
				overrides.push_back(optarg);
				break;
			case 'h':
			case '?':
			default:
				std::cout << "Usage: " << argv[0] << " [-h] [-D] [-c /path/to/supervisor.conf] [-s confparam=value...]" << std::endl;
				return 0;
		}
	}
#if 0
	int new_argc = argc - optind;
	char * new_argv[] = &argv[optind];
#endif

	super.basename(argv[0]);
#ifdef NAME_HEURISTIC
	std::string conffile_s;
	if(! conffile)
	{
		conffile_s = SYSCONFDIR;
		conffile_s += "/";
		conffile_s += super.basename();
		conffile_s += "/supervisor.conf";
		conffile = conffile_s.c_str();
	}
#endif
	if(! conffile)
		conffile = "supervisor.conf";

	config.default_context(&super);
	if(!config.read_file(conffile)) {
		std::cerr << "Error loading configuration file " << conffile << std::endl;
#ifdef USE_SYSLOG
		::syslog(LOG_ERR, "Error loading configuration file %s", conffile);
#endif
		return -1;
	}
	config.reset_context();
	for(std::vector<const char *>::const_iterator it = overrides.begin(); it != overrides.end(); ++it) {
		if(!config.parse_line(*it))
			std::cerr << "Can not parse config override line '" << *it << "'" << std::endl;
	}
	if(daemon)
		super.background(true);

	try {
		super.open_logs(false);
	}
	catch(std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
#ifdef USE_SYSLOG
		::syslog(LOG_ERR, "Got exception: %s", e.what());
#endif
		return -2;
	}

	if(super.background()) {
		daemonize();
	}
	int result = super.run();
	if(super.state() == Supervisor::ST_RESTART) {
#ifdef USE_SYSLOG
	::syslog(LOG_INFO, "Restarting");
#endif
		execvp(argv[0], argv);
	}
#ifdef USE_SYSLOG
	::syslog(LOG_INFO, "Exiting, rc: %d", result);
	closelog();
#endif
	return result;
}


