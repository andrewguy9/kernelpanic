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

my @tests = ();
if (@ARGV) {
        push @tests, @ARGV;
} else {
        print "Sourcing tests from regression.txt\n";
        open(REGRESSION_TESTS, "regression.txt");

        while( my $test = <REGRESSION_TESTS> ) {
                chomp( $test );
                unshift @tests, $test;
        }
}

my @tmp_tests = ();
for (my $i=0; $i<$runs; $i++) {
        push @tmp_tests, @tests;
}
@tests = @tmp_tests;

# Run tests
my $running = 0;
while (@tests || $running > 0)
{
        if ($running < $batchsize && @tests)
        {
                my $test = pop @tests;

                my $pid = fork();
                if ($pid == 0) {
                        runtest($test);
                } else {
                        print "Starting $test on $pid\n";
                        $running++;
                }
        } else {
                my $pid = wait;
                print "Reaped $pid with status $?\n";
                $running--;
        }
}
print "done\n";


sub get_stack {
        my ($program, $core) = @_;

        my $stack = "gdb $program $core\n";

        open GDB_OUTPUT, "gdb --command ./get_stack.gdb $program $core |"
                or die "Could not execute: $!";
        while(<GDB_OUTPUT>) {
                chomp $_;
                if($_ =~ /^#\d*\s*0x([0-9]|[a-f])* in .* at .*:\d*$/) {
                        $stack .= "$_\n";
                }
        }
        close GDB_OUTPUT;

        return $stack;
}

my $test_name;
my $test_passed;
my $test_pid;
sub runtest
{

        $test_name = shift @_;
        $test_passed = 0;

        #set up a timer to break us out after some time.
        local $SIG{ALRM} = sub{
                $test_passed = 1;
                        kill 'INT', $test_pid;
        };
        alarm $timeout;

        $test_pid = fork;
        if($test_pid == 0) {
                exec "./$test_name";
                die "failed to start $test_name\n";
        }

        my $dead = wait;
        if($test_passed) {
                exit(0);
        } else {
                my $core = "./$test_name.$test_pid.core";
                move("/cores/core.$test_pid", "$core");
                print "Core left at $core\n";
                print get_stack($test_name, $core);
                exit(1);
        }
}
