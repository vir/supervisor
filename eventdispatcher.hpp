#ifndef EVENTDISPATCHER_HPP_INCLUDED
#define EVENTDISPATCHER_HPP_INCLUDED
#include <map>
#include <queue>

class AsyncReader
{
	public:
		virtual int can_read(int fd)=0;
};

class ChildProc
{
	public:
		virtual void dead(int status)=0;
		virtual int kill(int signal)=0;
};

class TimerHandler
{
	public:
		virtual void timeout()=0;
};

class EventDispatcher
{
	private:
		bool m_ok;
		std::queue< std::pair<int, int> > m_deads;
		std::map<int, AsyncReader *> m_readers;
		std::map<int, ChildProc *> m_children;
		std::multimap<int, TimerHandler *> m_timeouts;
		static void sigchldhandler(int signum);
		void note_death(int pid, int status) { m_deads.push(std::pair<int, int>(pid, status)); }
	public:
		void register_reader(int fd, AsyncReader * reader);
		void unregister_reader(int fd);
		void register_chldproc(int pid, ChildProc * childproc);
		void unregister_childproc(int pid);
		void register_alarm(int when, TimerHandler * handler);
		int run();
};

extern EventDispatcher disp;

#endif /* EVENTDISPATCHER_HPP_INCLUDED */

