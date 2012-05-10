#!/bin/sh
#
# Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
# License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
#

D=/home/vir/supervisor
PIDFILE=$D/supervisor.PID

start()
{
	sudo -u vir /bin/sh -c "$D/supervisor </dev/null >/dev/null 2>/dev/null &"
	PID=$!
	echo "$PID" > $PIDFILE
}

stop()
{
	echo "NOT WORKING"
	rm $PIDFILE
	exit 100
	if [ -f $PIDFILE ]
	then
		PID=`head -1 $PIDFILE`
		kill -INT -$PID
	fi
}

check()
{
	if [ -f $PIDFILE ]
	then
		PID=`head -1 $PIDFILE`
		return 0
		return kill -0 $PID
	else
		return 1
	fi
}

case $1 in
	start)
		if check
		then
			echo "Process $PID is (still) alive. Kill it or remove $PIDFILE"
			exit 1
		fi
		start
		;;
	stop)
		stop
		;;
	restart)
		stop
		start
		;;
	*)
		echo "Usage: $0 start|stop|restart"
		;;
esac


