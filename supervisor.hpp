#ifndef SUPERVISOR_HPP_INCLUDED
#define SUPERVISOR_HPP_INCLUDED

#include <string>
#include "eventdispatcher.hpp"
#include "family.hpp"
#include "config.hpp"

class Supervisor:public ConfTarget
{
	private:
		std::map<std::string, Family *> m_fams;
		std::string m_logdir;
		std::string m_pidfile;
		bool m_background;
	protected:
		void autostart();
	public:
		Supervisor():m_logdir("."),m_background(false) { }
		virtual ~Supervisor() { }
		const std::string & logdir() const { return m_logdir; }
		bool background() const { return m_background; }
		void logdir(const std::string & d) { m_logdir = d; }
		Family * create_family(const std::string & name);
		int run();
		void shutdown();
		virtual bool configure(const std::string & var, const std::string & value);
		virtual ConfTarget * confcontext(const std::string & ctx, bool brackets);
};

extern Supervisor super;

#endif /* SUPERVISOR_HPP_INCLUDED */

