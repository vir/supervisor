
				  supervisor
				  ==========

Starts and restarts several "daemon" scripts with stdout and stderr redirected
into log files with timestamps added.


			    supervisor.conf sample
			    ----------------------

    logdir = /home/vir
    pidfile = ./supervisor.pid
    
    [first]
    cmd = ./tricky.pl first
    autostart = yes
    autorestart = yes
    restartdelay = 5
    log.timestamp = yes
    log.microseconds = yes
    
    [second]
    cmd = ./tricky.pl second
    autostart = yes
    autorestart = no


			configuration file parameters
			-----------------------------

`logdir` -- log files directory

`pidfile` -- pathname of file with supervisor daemon process identifier

`cmd` -- command line to start

`autostart` -- start command at supervisor startup

`autorestart` -- restart failed process after `restartdelay` seconds

`log.timestamp` -- add or not timestamp to log entries

`log.microseconds` -- timestamp microseconds precision


				 alternatives
				 ============

[supervisord (python-bassed)](http://supervisord.org/)

[daemontools](http://cr.yp.to/daemontools.html)

[launchd (Mac OS X origins)](http://en.wikipedia.org/wiki/Launchd)

[runit](http://smarden.org/runit/)

