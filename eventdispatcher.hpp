/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
#ifndef EVENTDISPATCHER_HPP_INCLUDED
#define EVENTDISPATCHER_HPP_INCLUDED
#include <map>
#include <queue>
#include <signal.h>

class AsyncReader
{
	public:
		virtual ~AsyncReader();
		virtual int can_read(int fd)=0;
};

class ChildProc
{
	public:
		virtual ~ChildProc();
		virtual void dead(int status)=0;
		virtual int kill(int signal)=0;
};

class TimerHandler
{
	public:
		virtual ~TimerHandler();
		virtual void timeout()=0;
};

class SignalHandler
{
	public:
		virtual ~SignalHandler();
		virtual void signal(int signum)=0;
};

class EventDispatcher
{
	private:
		bool m_ok;
		std::queue< std::pair<int, int> > m_deads;
		std::queue<int> m_sigq;
		std::map<int, AsyncReader *> m_readers;
		std::map<int, ChildProc *> m_children;
		std::multimap<int, TimerHandler *> m_timeouts;
		std::map<int, SignalHandler *> m_signals;
		static void sigchldhandler(int signum);
		static void signalhandler(int signum);
		void note_death(int pid, int status) { m_deads.push(std::pair<int, int>(pid, status)); }
	public:
		void register_reader(int fd, AsyncReader * reader);
		void unregister_reader(int fd);
		void register_chldproc(int pid, ChildProc * childproc);
		void unregister_childproc(int pid);
		void register_alarm(int when, TimerHandler * handler);
		void unregister_alarm(TimerHandler * handler);
		void register_signal(int signum, SignalHandler * handler);
		int run();
		void stop() { m_ok = false; }
};

extern EventDispatcher disp;

#endif /* EVENTDISPATCHER_HPP_INCLUDED */

