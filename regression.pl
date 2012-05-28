#!/usr/bin/perl

use warnings;
use strict;
use File::Copy;
use List::Util qw[min max];
use Getopt::Long;

#Get options
my $timeout = 60;
my $batchsize = 1;
my $runs = 1;

# Getopt::Long::Configure ('bundling_override');
GetOptions (
        'runs=i' => \$runs,
        'batch=i' => \$batchsize,
        'time=i' => \$timeout,
);

print "Timeout $timeout\n";
print "BatchSize $batchsize\n";
print "Runs $runs\n";

#Set STDOUT to flush right away
local $| = 1;

my %cores = (); # Key is test, value is pid.

#Get regression tests to run.
open(REGRESSION_TESTS, "regression.txt");

my @tests = ();
while( my $test = <REGRESSION_TESTS> ) {
        chomp( $test );
        unshift @tests, $test;
}

for (my $i=0; $i<$runs; $i++) {
        push @tests, @tests;
}

# Run tests
print "Setting up batch - - - - - - - - - - - - - - - - - - - - - - - - \n" if $batchsize != 1;
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
                                push @failures, "Test $kids{$dead}($dead)... FAILED!!!";
                                $cores{$kids{$dead}} = $dead;
                        }
                        delete $kids{$dead};
                } # While kids still around
                my @summary = (@successes, @failures);
                @summary = sort @summary;
                for my $line (@summary) {
                        print "$line\n";
                }
                print "Setting up batch - - - - - - - - - - - - - - - - - - - - - - - - \n" if $batchsize != 1;
                local $SIG{ALRM} = undef;
                @successes = ();
                @failures = ();
                %kids = ();
                $should_wait = min($batchsize, scalar @tests);
                $passed = 0;
        } # Should Wait
}

my @stacks = ();
print "all done testing, gathering cores\n";
for my $test (sort keys %cores) {
        my $core = "./$test.$cores{$test}.core";
        move("/cores/core.$cores{$test}", "$core");
        print "Core left at $core\n";
        push @stacks, get_stack($test, $core);
}

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
