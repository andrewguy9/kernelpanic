#!/usr/bin/perl

@pids = ();

#install signal handlers 


#run regression tests.
open(REGRESSION_TESTS, "regression.txt");

system "ulimit -c unlimited";

while( $test = <REGRESSION_TESTS> ) {

	#start the current test.
	chomp( $test );
	print "Starting $test\n";
	$pid = fork();

	if(not defined $pid) {
		print "failed to fork\n";
	} elsif ($pid == 0) {
		print "i'm a child, running $test\n";
		exec "./$test";
		die "failed to start $test\n";
	} else {
		print "i'm the parent, adding $pid to list\n";
		push(@pids, $pid);
	}
}

print "all processes running\n";
sleep(10);

foreach $test (@pids) {

	print "killing $test\n";
	kill 9, $test;
}

print "all done\n";
