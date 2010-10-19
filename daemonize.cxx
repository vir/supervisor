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
		exit(0);
	if(pid < 0) {
		perror("fork");
		exit(-1);
	}

	signal(SIGTSTP,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);

	sid = setsid();
	if(sid < 0)
		exit(-2);

	chdir("/");

	close(0);
	close(1);
	close(2);
}

