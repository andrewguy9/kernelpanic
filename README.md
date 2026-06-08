About
===

Kernel Panic is a small, real-time kernel originally designed as the software for Team Panic's entry to the IEEE Micromouse competition, which won the undergraduate division at UC Davis in 2007. The kernel is mainly developed by Andrew Thomson with some bits by Laine Walker-Avina. The kernel is released under the terms of the GPLv2.

Design Philosophy:
===

**No kernel-side allocation.** Many kernels allocate internally, which means allocation failure must be handled on the kernel path — a significant source of complexity. Kernel Panic takes a different approach: the kernel never allocates. Every kernel object — threads, stacks, locks, synchronization primitives, pipe buffers — is allocated by the caller and passed in. This makes allocation failure impossible on the kernel path and eliminates an entire class of error handling complexity.

**Locking contexts decoupled from thread blocking.** Most kernels conflate lock acquisition with blocking the calling thread. In Kernel Panic, locking is associated with a `LOCKING_CONTEXT` rather than implicitly with the calling thread. Passing `NULL` as the context uses the locking context embedded in the running thread, giving the familiar blocking behavior:

```c
// Blocking: thread sleeps until the semaphore is available
SemaphoreDown(&lock, NULL);
```

Alternatively, a caller can provide an explicit `LOCKING_CONTEXT` initialized with non-blocking wake/block functions and poll until the context reports acquired:

```c
// Non-blocking: thread continues running and polls for acquisition
struct LOCKING_CONTEXT context;
LockingInit(&context, LockingBlockNonBlocking, LockingWakeNonBlocking);
SemaphoreDown(&lock, &context);
while (!LockingIsAcquired(&context));
```

This pattern is demonstrated in `test/test5.c` (semaphore), `test/test3.c` (resource lock), and `test/test12.c` (signal). Because all synchronization is expressed through `LOCKING_CONTEXT` objects, this model also opens the door to a `LockingWaitAny`-style primitive: given a set of locking contexts, block until any one of them is acquired and return which one fired. This would allow a single thread to wait on multiple locks simultaneously without dedicating a thread to each. This was never implemented; see Unfinished Work.

**Build system with explicit flavor parameterization.** The build system uses checked (debug) and free (optimized) build flavors, parameterized by both architecture and build type. Object files are named `{arch}_{flavor}_{target}_{module}.o` (e.g. `pc_dbg_kern__scheduler.o`), allowing multiple build flavors to coexist in-tree without clobbering each other. The build is a single non-recursive make invocation with all dependencies visible at once, avoiding the classic recursive-make dependency tracking problems.

Examples:
===

**Pipe: producer/consumer with blocking IO**

Buffers are caller-allocated. The pipe holds no memory of its own. Writers block when the pipe is full; readers block when it is empty. See `test/test2.c` for a full multi-producer multi-consumer example.

`PipeRead` / `PipeWrite` transfer up to `size` bytes and return the count transferred — they may return short. `PipeReadStruct` / `PipeWriteStruct` atomically transfer exactly `size` bytes, making them suitable for passing fixed-size C structs between threads without torn reads or writes.

```c
// Setup: caller owns all memory
char ring[1024];
struct PIPE pipe;
PIPE_READ reader;
PIPE_WRITE writer;
PipeInit(ring, sizeof(ring), &pipe, &reader, &writer);

// Producer — holds write lock until all sizeof(msg) bytes are written
PipeWriteStruct((char *)&msg, sizeof(msg), writer);

// Consumer — holds read lock until all sizeof(msg) bytes are read; never sees a partial struct
struct MyMsg received;
PipeReadStruct((char *)&received, sizeof(received), reader);
```

**Socket: bidirectional communication**

A socket is two pipes. Each endpoint gets a handle with a read side pointing to one pipe and a write side pointing to the other. See `test/test10.c` and `test/test11.c`.

```c
char buf1[256], buf2[256];
struct SOCKET socket;
struct SOCKET_HANDLE h1, h2;
SocketInit(buf1, sizeof(buf1), buf2, sizeof(buf2), &socket, &h1, &h2);

// h1 writes to h2's read side, and vice versa
SocketWriteStruct(message, len, &h1);
SocketReadStruct(buf, len, &h2);
```

**Coroutine: resumable generator**

Coroutines use caller-provided stacks and save/restore execution context via `CoroutineYield` / `CoroutineNext`. See `test/test15.c`.

```c
void RangeRoutine(void *params, void *result, struct COROUTINE_CONTEXT *yield) {
    struct RANGE_PARAMS *p = params;
    struct RANGE_RESULT *r = result;
    for (INDEX cur = p->Low; cur < p->High; cur += p->Step) {
        r->Last = cur;
        CoroutineYield(yield);  // suspend here; resume on CoroutineNext()
    }
}

struct COROUTINE range;
CoroutineInit(RangeRoutine, &params, &result, stack, STACK_SIZE, &range);
while (CoroutineNext(&range) != COROUTINE_DONE) {
    // result.Last holds the current value
}
```

Testing:
===

**Stress tests** (`stress.txt`, run via `./stress`) are the primary correctness tests for the kernel. Each is a multi-threaded program that exercises a kernel subsystem under concurrent load — semaphores, pipes, sockets, barriers, signals, workers, etc. They run for a fixed time window and pass if the kernel does not panic or crash. They verify that the system stays up under contention but do not verify that specific operations produced correct results.

**Performance tests** (`performance.txt`, run via `./perf`) run both debug (`pc_dbg`) and release (`pc_fre`) builds to natural completion and record CPU time. Results are written to `perf.csv`. `perf_report` imports that CSV into SQLite on the host and compares average runtime between two branches, making it possible to detect regressions across commits.

**Coverage** is collected on Darwin using LLVM's instrumentation (`-fprofile-instr-generate -fcoverage-mapping`). `compute_coverage.sh` merges per-run `.profraw` files and produces an HTML report. Results are written to `coverage.csv`. `coverage_report` imports that CSV into SQLite on the host and diffs region, function, and line coverage percentages between two branches, showing only files where coverage changed.

**`runtests.sh`** ties it all together: checks out each branch, builds both flavors, runs both suites, and produces performance and coverage comparison reports. It requires a clean workspace so dirty files do not contaminate the commit attribution in the result CSV.

**Data structure tests** (`utils/linktest.c`, `treetest.c`, `bitmaptest.c`, `bhamtest.c`) and the **micromouse simulation** (`umouse/movetest.c`, `maptest.c`) are interactive shells — they accept input, manipulate state, and print visual representations of the data structures or maze. They are useful for manual exploration but are not automated: there are no recorded test cases, no expected outputs, and no pass/fail results. Correctness verification was done by eye at the time of writing and that knowledge was never recorded.

**Shortcomings.** The stress tests can show that the system survives but cannot prove that a specific concurrent interleaving produced the correct result. There are no per-function unit tests for the kernel subsystems — scheduler, locking primitives, pipe semantics — so a logic error that does not cause a crash or panic may go undetected. The interactive data structure tests have no automated equivalent, meaning the boundary conditions that were manually verified are unknown and cannot be re-run. The maze navigation algorithm in `movetest.c` is the most complex part of the application code and has no automated correctness tests at all.

Unfinished Work:
===

**LockingWaitAny / LockingWaitAll.** Two primitives operating on a set of `LOCKING_CONTEXT` objects: `LockingWaitAny` completes when any one of them is acquired; `LockingWaitAll` completes when all of them are acquired. Both would themselves accept a `LOCKING_CONTEXT`, so the caller chooses whether the wait is blocking or non-blocking, composing uniformly with the rest of the locking system.

**Non-blocking IO.** The locking framework supports non-blocking lock acquisition across all synchronization primitives, but the IO layer (pipes, sockets) was never redesigned to take advantage of this.

The obstacle is that a single IO operation acquires multiple locks in sequence. A pipe read must acquire `EmptyLock`, then `Mutex`, then conditionally release `FullLock`. If the operation is non-blocking and the first acquisition fails, it must return to the caller — but on the next call it cannot simply restart from the top, because it may already hold some locks. The operation needs to be resumable from the exact point it suspended, with lock state preserved across the return.

In C there is no native way to suspend and resume a function mid-execution. The coroutine system was written in part to explore whether coroutines could serve as that mechanism — a coroutine could yield at each lock acquisition point and be driven forward when the lock became available, which is recognizable today as async/await. In practice the IO layer was never redesigned around this model.

Kernel Features:
===

**Threading**
* Preemptive scheduler with priority-based thread selection
* Caller-provided thread struct, stack, and stack size — no kernel allocation
* Thread join / release
* Thread-local storage
* Coroutines — stackful, caller-provided stack, yield / next interface
* Worker queues — a dedicated thread that drains a queue of work item callbacks

**Synchronization**
* Mutex — non-blocking, returns success/failure
* Semaphore — counting, blocking
* Resource (dual-mode lock) — shared/exclusive, starvation-safe
* Barrier — blocks N threads until all have arrived
* Signal — binary flag that threads can wait on
* Generation — monotonic counter; threads can wait for a specific value, used for change notification

**Locking Framework**
* `LOCKING_CONTEXT` — unified pre-allocated token for blocking on any primitive
* `HANDLER_OBJECT` — generic callback with lifecycle state machine (ready → queued → running → finished); supports re-registration for periodic use

**Interrupt Handling**
* Layered IRQ levels: `CRIT`, `SOFT`, `SERIAL_WRITE`, `SERIAL_READ`, `TIMER`, `WATCHDOG`, `RESET`
* Hard interrupt registration
* Critical interrupt handler — highest priority deferred work
* Soft interrupt handler — lower priority deferred work
* Deferred interrupt processing — ISRs are kept minimal; work is pushed to handlers

**IO**
* Pipe — unidirectional byte stream, blocking, backpressure via leaked semaphores, caller-provided buffer
* Socket — bidirectional channel built from two pipes, caller-provided buffers
* Serial — kernel-owned ring buffer, ISR-driven, backpressure when full, copy-on-read to caller buffer

**Time**
* Current time query
* Thread sleep
* Timer — one-shot or periodic callback via `HANDLER_OBJECT`

**Watchdog**
* Programmable timeout
* Multi-subsystem liveness tracking — all registered subsystems must check in before timeout expires

**Startup / Panic**
* `KernelInit` / `KernelStart`
* `KernelPanic` — captures source location, halts via HAL

**Embedded Lisp**
* `minilisp` — a Lisp interpreter with a copying garbage collector, two-space heap (caller-provided memory), and a serial REPL thread

Utilities:
===

* Linked list — doubly-linked, intrusive
* Atomic list — lock-free singly-linked stack using double-width CAS, ABA-safe via generation counter
* Min-heap — weighted, intrusive, used by the timer subsystem
* AVL tree — weighted, intrusive, balanced BST
* Ring buffer — fixed-size circular buffer with caller-provided backing memory, copy-based read/write
* Bitmap — macro-based bit array with get/set/clear/toggle and first-set-bit scan
* Bresenham line algorithm — integer approximation, used for motor control
* Counter — monotonic counter with per-observer context for detecting changes
* Flags — bitmask manipulation macros
* Union link — single intrusive node usable in linked list, atomic list, or weighted tree
* Primes — primality test and prime enumeration
* Atomic primitives — compare-and-swap, double-width compare-and-swap, atomic get-and-and/or

Supported Architectures:
===

Today, Kernel Panic runs as a Unix process; signals and TTYs are used to simulate hardware interrupts and serial ports. This is useful for development because you get access to good compilers and debuggers.

The kernel originally ran on AVR (ATmega128) hardware for the Micromouse competition. By implementing your own HAL file, you can build Kernel Panic for your own embedded hardware.

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
Install uuid generator `sudo apt-get install uuid-runtime`.

RedHat
---
Install Perl which library: 'sudo yum install perl-File-Which' in RedHat.

Building:
===

* `make pc` builds kernel and tests in debug build mode.
* `make pc_fre` builds kernel and tests in release mode.
* Run the performance test suite:
```
./perf -h
Usage: ./perf [--runs i] [--batch i] [--time i] [--debugger s] [--coredir s] [--jobid s] [--help] <test> ...
```
* Run the stress test suite:
```
./stress -h
Usage: ./stress [--runs i] [--batch i] [--time i] [--debugger s] [--coredir s] [--jobid s] [--help] <test> ...
```
* Run the whole suite, and generate reports:
```
./runtests.sh -h
help: ./runtests.sh [-r runs] [-b batch_size] dst_branch [branch ...]
```

Ubuntu Linux:
---

Ubuntu uses apport for crash analysis. This prevents core dumps from being available for analysis.
To enable coredumps:
```
sudo su
echo %e.%p.core > /proc/sys/kernel/core_pattern
```

Mac OSX:
---
By default, `/cores` is not writable. Make it writable.
```
sudo chmod o+w /cores/
```

Debugging:
===

lldb
---

Panic uses signals to simulate hardware interrupts. LLDB captures signals and presents them to the user. Generally, when using LLDB you should disable signal handling so that Panic can do its work.

`process handle SIGNAL_X -n false -p true -s false`

In most cases you should ignore the following signals:
```
SIGINFO
SIGALRM
SIGUSR1
SIGUSR2
```

UNIX:
---

Exit Codes:
* `0` - Kernel shut down.
* `1` - Kernel was reset by reset button, or `SIGINT` on UNIX.
* Other - Kernel panicked. On UNIX, a crash is generated due to call to `abort()`.
