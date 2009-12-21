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

Supervisor super;

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

void Supervisor::shutdown()
{
	std::map<std::string, Family *>::iterator it;
	for(it = m_fams.begin(); it != m_fams.end(); it++) {
		it->second->stop();
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

int Supervisor::run()
{
	Pidfile pidfile(m_pidfile.c_str());
	autostart();
	return disp.run();
}

int main(int argc, char * argv[])
{
	const char * conffile = "supervisor.conf";
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
	argc -= optind;
	argv += optind;

	config.default_context(&super);
	if(!config.read_file(conffile)) {
		std::cerr << "Can not open config file " << conffile << std::endl;
		return -1;
	}
	if(super.background() || daemon) {
		daemonize();
	}
	return super.run();
}


