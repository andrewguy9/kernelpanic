#!/usr/bin/perl

use File::Copy;

%pids = {};
$children = 0;

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
		$pids{ $pid } = $test;
		$children = $children + 1;
	}
}

print "all processes running\n";

while( $children > 0 ) {
	$dead = wait;
	if ($dead == -1) {
		die "wait returned -1"
	} else {
		print "pid $dead  died, which is test $pids{$dead} is dead\n";
		$children = $children -1;
		print "moving /cores/core.$dead to ./$pids{$dead}.core\n";
		move("/cores/core.$dead", "./$pids{$dead}.core");
	}
}

print "all done\n";
