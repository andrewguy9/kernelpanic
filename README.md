About:
===

Kernel panic is a small, real-time kernel originally designed as the software for Team Panic’s entry to the IEEE Micromouse competition. The kernel is mainly developed by Andrew Thomson with some bits by Laine Walker-Avina. The kernel is released under the terms of the GPLv2.

Features:
===

* Small memory footprint (<2k RAM, <30k flash with kernel and application code)
* Differed interrupt processing for real-time performance
* Support for semaphores, mutex, Dual mode locks, gather scatter locks, and signals
* Pre-emptive Thread Scheduling
* Worker threads
* Programmable Watchdog.
* Machine independent implementation (all machine specifics are in one file)
* Embedded lisp interpreter and serial repl.

Supported Architectures:
===

Today, panic only works as a unix process; signals and TTYs are used to simulate hardware interrupts and serial ports. This is useful for development because you get access to good compilers and debuggers.

By implementing your own hal file, you can build panic for your embedded hardware.

Supported Process OSes:
---
Mac OSX
Linux

Installing Dependencies:
===

Build requires GCC or CLang.

Tests require:
* Python 2.7
* Perl
* Bash
* SQLite

Libraries:

Debian
---
Install Perl which library: `sudo apt-get install libfile-which-perl`.

RedHat
---
Install Perl which library: 'sudo yum install perl-File-Which' in RedHat.

Building:
===

* `make pc` builds kernel and tests in debug build mode.
* `make pc_fre` builds kernel and tests in release mode.
* `./regression -r $runs -b $threads -t $seconds` will run regression tests.
* `./benchmark -r $runs -b $threads branch1 branch2` will run performance tests and reports.

Ubuntu Linux:
---

Ubuntu uses apport for crash analysis. This prevents core dumps from being available for analysis.
To enable coredumps:
```
sudo su
echo %e.%p.core > /proc/sys/kernel/core_pattern
```

Debugging:
===

lldb
---

Panic uses signals to simulate hardware interrupts. LLDB captures signals and presents them to the user. Generally, when using LLDB you should disable signal handling so that Panic can do it's work.

`process handle SIGNAL_X -n false -p true -s false`

In most cases you should ignore the following signals:
```
SIGINFO
SIGALRM
SIGUSR1
SIGUSR2
```
