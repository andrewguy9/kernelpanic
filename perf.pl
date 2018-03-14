#!/usr/bin/perl

use warnings;
use strict;
use Getopt::Long;

my $batchsize = 1;
my $runs = 1;

# Getopt::Long::Configure ('bundling_override');
GetOptions (
        'runs=i' => \$runs,
        'batch=i' => \$batchsize,
);

print "BatchSize $batchsize\n";
print "Runs $runs\n";

my $jobid = shift @ARGV;
my @tests = ();
push @tests, @ARGV;

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

my $test_name;
my $test_passed;
my $test_pid;
sub runtest
{

        $test_name = shift @_;
        $test_passed = 0;

        $test_pid = fork;
        if($test_pid == 0) {
                exec "./perf.sh $jobid $test_name";
                die "failed to start $test_name\n";
        }

        my $status = 0;
        my $msg = "Success";

        my $dead = wait;
        $status = $?;

        print "Test $test_name($test_pid)... $msg\n" if $status;
        exit($status);
}
