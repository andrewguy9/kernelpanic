#!/usr/bin/perl

use warnings;
use strict;

#Set STDOUT to flush right away
local $| = 1;

my @failures = ();

use File::Copy;

#Get timeout for tests.
my $timeout = $ARGV[0];
if(!$timeout) {
	print "Assuming timeout of 60\n";
	$timeout = 60;
}

#Get regression tests to run.
open(REGRESSION_TESTS, "regression.txt");

#Make sure cores are generated.
system "ulimit -c unlimited";

while( my $test = <REGRESSION_TESTS> ) {

	#start the current test.
	chomp( $test );

	my $killed = 0;
	#print "Starting $test\n";
	my $pid = fork();

	if(not defined $pid) {
		print "failed to fork\n";
	} elsif ($pid == 0) {
		#print "i'm a child, running $test\n";
		exec "./$test";
		die "failed to start $test\n";
	} else {
		#print "i'm the parent, adding $pid to list\n";
		print "$test running as pid $pid...";

		#set up a timer to break us out after some time.
		local $SIG{ALRM} = sub{
			$killed = 1;
			kill 'INT', $pid;
			#die 'alarm';#dont know why he wants to die!
		};
		alarm $timeout;

		my $kid = wait;
		local $SIG{ALRM} = undef;
		if ($kid == -1) {
			die "wait returned -1"
		} else {
			if($killed) {
				print " passed!\n";
			} else {
				print " ***died unexpectedly*** ";
				my $core = "./$test.$kid.core";
				push @failures, "gdb $test $core";
				move("/cores/core.$kid", "$core");
				print " Core left at $core ";
				print "\n";
			}
		}
	}
}

print "all done testing\n";

print "\n\n";
print "Summary:\n";
print "-------------------------------------------------------------------------------\n";
if(@failures) {
	for my $cur (@failures) {
		print "$cur\n";
	}
} else {
	print "All tests passed!\n";
}
print "-------------------------------------------------------------------------------\n";
	print "done\n";
