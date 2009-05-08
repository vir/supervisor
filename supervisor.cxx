#include "supervisor.hpp"

#include <string>
#include <iostream>
#include <map>

#include "family.hpp"
#include "eventdispatcher.hpp"
#include "config.hpp"

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
 
bool Supervisor::configure(const std::string & var, const std::string & value)
{
	if(var == "logdir") {
		m_logdir = value;
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
 
int main(int argc, char * argv[])
{
//	Supervisor s;
//	s.add(new Family(&s, "first"))->cmd("./tricky.pl first try");
//	s.add(new Family(&s, "second"))->cmd("./tricky.pl second and last");
	config.default_context(&super);
	config.read_file("supervisor.conf");
	super.autostart();
	return disp.run();
}


