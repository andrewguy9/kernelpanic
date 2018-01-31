About:
===

Kernel panic is a small, real-time kernel originally designed as the software for Team Panic’s entry to the IEEE Micromouse competition. The kernel is mainly developed by Andrew Thomson with some bits by Laine Walker-Avina. The kernel is released under the terms of the GPLv2.

Features:
===

* Small memory footprint (<2k RAM, <30k flash with kernel and application code)
* Differed interrupt processing for real-time performance
* Support for semaphores, mutexs, Dual mode locks, gather scatter locks, and signals
* Pre-emptive Thread Scheduling
* Worker threads
* Programmable Watchdog.
* Machine independent implementation (all machine specifics are in one file)

Supported Architectures:
===

Today, panic only works as a unix process; signals and TTYs are used to simulate hardware interrupts and serial ports. This is useful for development because you get access to good compilers and debuggers.

By implementing your own hal file, you can build panic for your embedded hardware.

Supported Process OSes:
---
Mac OSX
Linux

Building:
===

Ubuntu Linux:
---

Ubuntu uses apport for crash analysis. This prevents core dumps from being available for analysis.
To enable coredumps:
```
sudo su
echo %e.%p.core > /proc/sys/kernel/core_pattern
```
