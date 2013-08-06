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
	int daemon = 0;
	int ch;
	while ((ch = getopt(argc, argv, "?hc:D")) != -1) {
		switch (ch) {
			case 'c':
				conffile = optarg;
				break;
			case 'D':
				daemon++;
				break;
			case 'h':
			case '?':
			default:
				std::cout << "Usage: " << argv[0] << " [-h] [-D] [-c /path/to/supervisor.conf]" << std::endl;
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
		std::cerr << "Can not open config file " << conffile << std::endl;
		return -1;
	}
	if(daemon)
		super.background(true);

	if(super.background()) {
		daemonize();
	}
	int result = super.run();
	if(super.state() == Supervisor::ST_RESTART) {
		execvp(argv[0], argv);
	}
	return result;
}


