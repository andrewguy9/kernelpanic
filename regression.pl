#!/usr/bin/perl

use List::Util qw[min max];
use warnings;
use strict;

#Set STDOUT to flush right away
local $| = 1;

my @stacks = ();

use File::Copy;

#Get timeout for tests.
my $timeout = $ARGV[0];
if(!$timeout) {
        print "Assuming timeout of 60\n";
        $timeout = 60;
}
#Get batchsize for tests
my $batchsize = 7;

#Get regression tests to run.
open(REGRESSION_TESTS, "regression.txt");

my @tests = ();
while( my $test = <REGRESSION_TESTS> ) {
        chomp( $test );
        unshift @tests, $test;
}

# Run tests
print "Setting up batch - - - - - - - - - - - - - - - - - - - - - - - - \n";
my %kids = ();
my $should_wait = min($batchsize, scalar @tests);
my $passed = 0;
my @successes = ();
my @failures = ();
while (@tests) {
        my $test = pop @tests;

        my $pid = fork();
        if(not defined $pid) {
                die "failed to fork\n";
        } elsif ($pid == 0) {
                exec "./$test";
                die "failed to start $test\n";
        }

        #print "i'm the parent, adding $pid to list\n";
        $kids{ $pid } = $test;

        $should_wait--;
        #We have a full batch.
        if( $should_wait == 0 ) {
                #set up a timer to break us out after some time.
                local $SIG{ALRM} = sub{
                        $passed = 1;
                        for my $kid (keys %kids) {
                                kill 'INT', $kid;
                        }
                };
                alarm $timeout;

                #wait for all kids to finish.
                while( %kids ) {
                        my $dead = wait;
                        if ($dead == -1) {
                                die "wait returned -1"
                        }
                        if($passed) {
                                push @successes, "Test $kids{$dead}($dead)... Passed";
                        } else {
                                my $core = "./$kids{$dead}.$dead.core";
                                push @failures, "Test $kids{$dead}($dead)... FAILED!!!";
                                move("/cores/core.$dead", "$core");
                                print "Core left at $core\n";
                                push @stacks, get_stack($kids{$dead}, $core);
                        }
                        delete $kids{$dead};
                } # While kids still around
                my @summary = (@successes, @failures);
                @summary = sort @summary;
                for my $line (@summary) {
                        print "$line\n";
                }
                print "Setting up batch - - - - - - - - - - - - - - - - - - - - - - - - \n";
                local $SIG{ALRM} = undef;
                @successes = ();
                @failures = ();
                %kids = ();
                $should_wait = min($batchsize, scalar @tests);
                $passed = 0;
        } # Should Wait
}

print "all done testing\n";

print "\n\n";
print "Summary:\n";
print "-"x80 . "\n";
if(@stacks) {
        for my $stack (@stacks) {
                print $stack;
                print "-"x80 . "\n";
        }
} else {
        print "All tests passed!\n";
}
print "done\n";


sub get_stack {
        my ($program, $core) = @_;

        my $stack = "gdb $program $core\n";

        open GDB_OUTPUT, "gdb --command ./get_stack.gdb $program $core |" or die "Could not execute: $!";
        while(<GDB_OUTPUT>) {
                chomp $_;
                if($_ =~ /^#\d*\s*0x([0-9]|[a-f])* in .* at .*:\d*$/) {
                        $stack .= "$_\n";
                }
        }
        close GDB_OUTPUT;

        return $stack;
}
