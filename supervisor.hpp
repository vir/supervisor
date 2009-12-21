#ifndef SUPERVISOR_HPP_INCLUDED
#define SUPERVISOR_HPP_INCLUDED

#include <string>
#include "eventdispatcher.hpp"
#include "family.hpp"
#include "config.hpp"

class Supervisor:public ConfTarget, public SignalHandler, public TimerHandler
{
	public:
		enum State { ST_RUN, ST_SHUTDOWN, ST_RESTART };
	private:
		State m_state;
		std::map<std::string, Family *> m_fams;
		std::string m_logdir;
		std::string m_pidfile;
		bool m_background;
	protected:
		void autostart();
		virtual void signal(int signum);
		virtual void timeout();
	public:
		Supervisor():m_state(ST_RUN),m_logdir("."),m_background(false) { }
		virtual ~Supervisor() { }
		State state() const { return m_state; }
		const std::string & logdir() const { return m_logdir; }
		bool background() const { return m_background; }
		void logdir(const std::string & d) { m_logdir = d; }
		Family * create_family(const std::string & name);
		int run();
		void shutdown(bool restart = false);
		unsigned int count_running() const;
		virtual bool configure(const std::string & var, const std::string & value);
		virtual ConfTarget * confcontext(const std::string & ctx, bool brackets);
};

extern Supervisor super;

#endif /* SUPERVISOR_HPP_INCLUDED */

