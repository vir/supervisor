/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
#include "daemonize.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

void daemonize()
{
	int pid, sid;

	pid = fork();
	if(pid > 0)
		_exit(0);
	if(pid < 0) {
		perror("fork");
		exit(-1);
	}

	sid = setsid();
	if(sid < 0)
		exit(-2);

	/* TODO: fork() again so the parent, (the session group leader), can exit.
	 * This means that we, as a non-session group leader, can never regain a
	 * controlling terminal. */

	signal(SIGTSTP,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);

	chdir("/");

	/*  umask(0) so that we have complete control over the permissions of
	 *  anything we write. We don't know what umask we may have inherited. [This
	 *  step is optional] */

	close(0);
	close(1);
	close(2);

	/* TODO: Establish new open descriptors for stdin, stdout and stderr. Even if you
	 * don't plan to use them, it is still a good idea to have them open. The
	 * precise handling of these is a matter of taste; if you have a logfile,
	 * for example, you might wish to open it as stdout or stderr, and open
	 * `/dev/null' as stdin; alternatively, you could open `/dev/console' as
	 * stderr and/or stdout, and `/dev/null' as stdin, or any other combination
	 * that makes sense for your particular daemon. */
}

