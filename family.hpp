#ifndef FAMILY_HPP_INCLUDED
#define FAMILY_HPP_INCLUDED

#include <string>
#include "eventdispatcher.hpp"
#include "config.hpp"

class Logger;
class Supervisor;
class ChildProcess;
class Family:public TimerHandler, public ConfTarget
{
	public:
		enum State { ST_RUN, ST_SHUTDOWN, ST_RESTART };
	private:
		State m_state;
		ChildProcess * m_active;
		std::string m_name;
		std::string m_cmd;
		bool m_autostart;
		bool m_autorestart;
		int m_restart_delay;
		Logger * m_logger;
		std::deque<std::string> m_logtail;
		unsigned int m_logtailsize;
		unsigned int m_shutdownphase;
		void shutdown(bool initial = true);
	public:
		Family(const std::string & name);
		virtual ~Family();
		std::string cmd() const { return m_cmd; }
		void cmd(const std::string & s) { m_cmd = s; }
		std::string name() const { return m_name; }
		State state() const { return m_state; }
		bool start();
		bool stop();
		bool restart();
		bool autostart();
		void celebrate_child_death(ChildProcess * cp, int status);
		void output(char stream, const std::string & s, bool forcetimestamp = false);
		virtual void timeout();
		virtual bool configure(const std::string & var, const std::string & value);
		virtual ConfTarget * confcontext(const std::string & ctx, bool brackets);
};

#endif /* FAMILY_HPP_INCLUDED */

