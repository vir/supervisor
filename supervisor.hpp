/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
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
		std::string m_basename;
	protected:
		void autostart();
		virtual void signal(int signum);
		virtual void timeout();
	public:
		Supervisor();
		virtual ~Supervisor() { }
		State state() const { return m_state; }
		const std::string & logdir() const { return m_logdir; }
		const std::string & basename() const { return m_basename; }
		bool background() const { return m_background; }
		void background(bool b) { m_background = b; }
		void logdir(const std::string & d) { m_logdir = d; }
		void basename(const char * exe);
		Family * create_family(const std::string & name);
		int run();
		void shutdown(bool restart = false);
		unsigned int count_running() const;
		void open_logs(bool reopen);
		virtual bool configure(const std::string & var, const std::string & value);
		virtual ConfTarget * confcontext(const std::string & ctx, bool brackets);
};

extern Supervisor super;

#endif /* SUPERVISOR_HPP_INCLUDED */

