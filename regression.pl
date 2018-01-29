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
my $debugger = 'gdb';

# Getopt::Long::Configure ('bundling_override');
GetOptions (
        'runs=i' => \$runs,
        'batch=i' => \$batchsize,
        'time=i' => \$timeout,
        'debugger=s' => \$debugger,
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
                        $running++;
                }
        } else {
                my $pid = wait;
                $running--;
        }
}
print "done\n";


sub get_stack {
        my ($program, $core) = @_;

        my $debug_cmd;
        if($debugger eq "gdb") {
          $debug_cmd = "gdb --command ./get_stack.gdb $program $core";
        } elsif($debugger eq "lldb") {
          $debug_cmd = "lldb --core $core --source ./get_stack.gdb";
        } else {
          die("Unrecognized debugger");
        }

        my $stack = "-" x 80;
        $stack .= "\n";
        $stack .= "$debug_cmd\n";

        open DBG_OUTPUT, "$debug_cmd |"
                or die "Could not execute: $!";
        while(<DBG_OUTPUT>) {
                chomp $_;
                $stack .= "$_\n";
        }
        close DBG_OUTPUT;

        $stack .= "-" x 80;
        $stack .= "\n";

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

        my $status = 0;
        my $msg = "Success";

        my $dead = wait;
        if(! $test_passed) {
                $status = 1;
                $msg = "FAILED!!!";
                my $core = "./$test_name.$test_pid.core";
                move("/cores/core.$test_pid", "$core");
                print "Core left at $core\n";
                print get_stack($test_name, $core);
        }

        print "Test $test_name($test_pid)... $msg\n" if $status;
        exit($status);
}
