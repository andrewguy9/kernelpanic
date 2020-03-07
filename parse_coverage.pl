#!/usr/bin/perl

use warnings;
use strict;
use Getopt::Long;

my $job;

GetOptions (
  'job=s' => \$job,
);

my $branch;
my $commit;
my $branch_output=`git branch -v`;
foreach (split(/\n/,$branch_output)) {
  if ($_ =~ /^\*\s+(\S+)\s+(\S+)/) {
    ($branch, $commit) = ($1,$2);
  }
}

my $git_status = `git status --porcelain`;
my @dirty;
foreach (split(/\n/, $git_status)) {
  if ($_ =~ /^ M (.*)/) {
    push(@dirty, $1);
  }
}
my $dirty_str = join(";", @dirty);

# Parse the report from STDIN.
my @stats = ();

# Consome the labels
my $labels = readline;
chomp($labels);
# Consume the dash header
readline;
# Consume the file stats...
while (my $file_stats = readline) {
  chomp($file_stats);
  if (not $file_stats) {
    last;
  }
  $file_stats =~ s/%//g;
  my @file_stats = split(/\s+/, $file_stats);
  push @stats, [@file_stats];
}
# Consume blank line
# Consumed by above last breakout.

# Consume no function warning
readline;

# Consume no coverage files...
while (my $file_stats = readline) {
  chomp($file_stats);
  if ($file_stats =~ /^-+$/) {
    last;
  }
  $file_stats =~ s/%//g;
  my @file_stats = split(/\s+/, $file_stats);
  $file_stats[3]=100.0;
  $file_stats[6]=100.0;
  $file_stats[9]=100.0;
  push @stats, [@file_stats];
}
# Consome the dashes footer
# Consumed by above last statement.

# Consume the total
my $file_stats = readline;
chomp($file_stats);
$file_stats =~ s/%//g;
my @file_stats = split(/\s+/, $file_stats);
push @stats, [@file_stats];

# Turn report into csv lines.
foreach (@stats) {
  print "$job,$branch,$commit,$dirty_str," . join(",", @$_) . "\n";
}
