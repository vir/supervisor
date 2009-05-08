#include "eventdispatcher.hpp"
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include <iostream>

#if 0
# define XDEBUG(a) a
#else
# define XDEBUG(s)
#endif

EventDispatcher disp;

void EventDispatcher::register_reader(int fd, AsyncReader * reader)
{
	XDEBUG(std::cout << "Adding reader " << reader << " (fd: " << fd << ")" << std::endl);
	m_readers[fd] = reader;
}

void EventDispatcher::unregister_reader(int fd)
{
	XDEBUG(std::cout << "Removing reader (fd: " << fd << ")" << std::endl);
	m_readers.erase(fd);
}

void EventDispatcher::register_chldproc(int pid, ChildProc * childproc)
{
	XDEBUG(std::cout << "Adding sigchld handller for pid " << pid << std::endl);
	m_children[pid] = childproc;
}

void EventDispatcher::unregister_childproc(int pid)
{
	XDEBUG(std::cout << "Removing sigchld handler for pid " << pid << std::endl);
	m_children.erase(pid);
}

void EventDispatcher::register_alarm(int when, TimerHandler * handler)
{
	XDEBUG(std::cout << "Adding alarm(" << when << ", " << handler << ")" << std::endl);
	if(when < 1000000)
		when += time(NULL);
	m_timeouts.insert(std::pair<int, TimerHandler *>(when, handler));
}

void EventDispatcher::sigchldhandler(int signum)
{
	std::cout << "Got SIGCHLD" << std::endl;
	int pid, status;
	while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		disp.note_death(pid, status);
	}
}

#include <stdio.h>
int EventDispatcher::run()
{
	int r;
	int maxfd = 0;
	fd_set rfds;
	struct timeval tv;
	struct sigaction new_action;
	new_action.sa_handler = sigchldhandler;
	sigemptyset (&new_action.sa_mask);
	new_action.sa_flags = 0;
	sigaction (SIGCHLD, &new_action, NULL);

	m_ok = true;
	while(m_ok) {
		FD_ZERO(&rfds);
		std::map<int, AsyncReader *>::iterator it;
		for(it = m_readers.begin(); it != m_readers.end(); it++) {
			FD_SET(it->first, &rfds);
			if(it->first > maxfd)
				maxfd = it->first;
		}
		tv.tv_sec = 30;
		tv.tv_usec = 0;
		if(m_timeouts.size()) {
			int now = time(NULL);
			int when = m_timeouts.begin()->first;
			tv.tv_sec = (now < when) ? when - now : 0;
		}

		r = select(maxfd + 1, &rfds, NULL, NULL, &tv);
		if (r == -1) {
//			perror("select()");
		}	else if (r) {
//			printf("Data is available now.\n");
		/* FD_ISSET(0, &rfds) will be true. */
			for(it = m_readers.begin(); it != m_readers.end(); it++) {
				if(FD_ISSET(it->first, &rfds))
					it->second->can_read(it->first);
			}
		} else {
//			printf("timeout\n");
		}

		/* dispatch signals */
		while(!m_deads.empty()) {
			int pid = m_deads.front().first;
			int status = m_deads.front().second;
			std::pair<int, int> res = m_deads.front();
			XDEBUG(std::cout << "Registered death of process " << res.first << ", status " << res.second << std::endl);
			std::map<int, ChildProc *>::iterator it = m_children.find(pid);
			if(it != m_children.end())
				it->second->dead(status);
			m_deads.pop();
		}

		/* dispatch timeouts */
		int now = time(NULL);
		while(m_timeouts.size() && m_timeouts.begin()->first <= now) {
			m_timeouts.begin()->second->timeout();
			m_timeouts.erase(m_timeouts.begin());
		}
	}
	return 0;
}


