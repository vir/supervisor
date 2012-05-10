#!/usr/bin/perl
# Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
# License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
use strict;
use warnings;
$|=1;
print "$0 @ARGV (pid $$) started\n";
my $t = 1 + int rand(3);
print "waiting $t seconds\n";
sleep $t;
for(my $i = int rand(20); $i > 0; $i--) {
	print " * $i bottles of beer on the wall\n";
}
print "$t seconds passed... it is time to die now...\n";
$t = rand(10);
print "rand(10)=$t\n";
if($t < 3) {
	warn "warning: setting alarm to two seconds\n";
	alarm(2);
	select(undef, undef, undef, 10);
} elsif($t < 7) {
	warn "warning: sending SIGHUP to myself\n";
	kill 1, $$;
	select(undef, undef, undef, 10);
} else {
	die "Testing plain old die()\n";
}

exit 33;
