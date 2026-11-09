*This project has been created as part of the 42 curriculum by roda-fon.*

# Codexion

## Description

Codexion is a concurrency simulation written in C with POSIX threads, a variant of the
classic *Dining Philosophers* problem.

`N` coders sit around a table with `N` USB dongles, one dongle between each pair of
neighbours. To **compile**, a coder needs both of their dongles at the same time. After
compiling they put the dongles back, **debug**, **refactor**, and then try to compile
again. A coder who does not start compiling within `time_to_burnout` ms of the start of
their last compile (or of the simulation) **burns out**, and the simulation stops.

The project adds a few constraints on top of the classic problem:

- **Dongle cooldown**: a released dongle stays unavailable for `dongle_cooldown` ms.
- **Fair arbitration**: when several coders want the same dongle, it is granted
  according to a scheduler: `fifo` (arrival order) or `edf` (Earliest Deadline First,
  where the deadline is `last_compile_start + time_to_burnout`).
- **A self-implemented priority queue (binary min-heap)** decides who is served next.
- **A separate monitor thread** detects burnout and announces it within 10 ms.
- **Serialized logs**: two messages never mix and nothing is printed after a burnout.

The simulation ends when a coder burns out, or when every coder has compiled at least
`number_of_compiles_required` times.

## Instructions

### Compilation

```sh
make        # builds ./codexion
make clean  # removes object files
make fclean # removes object files and the binary
make re     # full rebuild
```

The project compiles with `cc -Wall -Wextra -Werror -pthread`.

### Execution

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning |
|---|---|
| `number_of_coders` | number of coders (threads), which is also the number of dongles (≥ 1) |
| `time_to_burnout` | ms a coder can go without starting a compile |
| `time_to_compile` | ms spent compiling (holding two dongles) |
| `time_to_debug` | ms spent debugging |
| `time_to_refactor` | ms spent refactoring |
| `number_of_compiles_required` | the simulation stops once every coder has compiled this many times |
| `dongle_cooldown` | ms a dongle stays unavailable after being released |
| `scheduler` | `fifo` or `edf` |

All arguments are mandatory. Negative numbers, non-integers, values above `INT_MAX` and
unknown schedulers are rejected with a usage message on `stderr` and exit code `1`.

### Examples

```sh
./codexion 5 800 200 200 200 5 10 edf   # nobody should burn out
./codexion 4 410 200 200 200 5 0 fifo   # a coder burns out (~410 ms)
./codexion 1 800 200 200 200 3 0 fifo   # single coder: one dongle, burns out at ~800 ms
```

Output format (one line per state change):

```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

### Project layout

| File | Role |
|---|---|
| `include/codexion.h` | structures (`t_params`, `t_req`, `t_heap`, `t_dongle`, `t_coder`, `t_sim`) and prototypes |
| `src/main.c` | parses, initialises, creates/joins threads, cleans up |
| `src/parse.c` | argument validation |
| `src/init.c` / `src/cleanup.c` | creation and destruction of every mutex, condition variable and allocation |
| `src/time.c` | `now_ms()` and an interruptible precise `sleep_ms()` |
| `src/heap.c` / `src/heap_utils.c` | the binary min-heap (priority queue) |
| `src/dongle.c` | `dongles_take()` / `dongles_release()`: atomic acquisition of both dongles |
| `src/dongle_utils.c` | request keys (fifo/edf), the arbitration rule `can_go()` and the timed wait |
| `src/coder.c` | the coder thread routine |
| `src/monitor.c` | the monitor thread routine (burnout and completion detection) |
| `src/state.c` | the shared stop flag and serialized logging |

## Blocking cases handled

### Deadlock (Coffman's conditions)

A deadlock requires all four Coffman conditions: **mutual exclusion**, **hold and wait**,
**no preemption** and **circular wait**. The solution breaks **hold and wait**: a coder
takes **both dongles at once or none**. The check "are both of my dongles available?" and
the act of taking them both happen inside one critical section (`table_lock`), so a
coder never holds one dongle while waiting for the other. Without hold and wait, no
cycle of coders waiting on each other's dongles can form.

This also avoids *chains*. When a coder grabs one dongle and then waits for the second,
it blocks its neighbour, who blocks the next one, and so on. Coders end up compiling one
at a time and someone burns out even though the parameters are feasible (with the
earlier one-dongle-at-a-time design, `5 3000 200 200 200 10 400 fifo` burned out in
about half of the runs; it no longer does).

### Starvation and fair arbitration

Every dongle owns a **priority queue** (a binary min-heap) of pending requests, each
stored as `{id, key}`. A waiting coder joins the queues of both of its dongles with the
same key:

- `fifo`: `key` is a global ticket counter, so requests are served in arrival order.
- `edf`: `key = last_compile_start + time_to_burnout`, so the coder closest to burning
  out is served first.
- **Tie-break**: equal keys are ordered by coder id, which makes the policy fully
  deterministic.

A coder may take its two dongles (`can_go()`) when both are free, both cooldowns are
over, and neither dongle is **reserved by a higher-priority neighbour**. The neighbour at
the top of a dongle's queue reserves it only if it is also at the top of its *other*
dongle's queue, which means it is next in line for both of its dongles. A neighbour that
is first on one queue but itself blocked on the other does not reserve anything, so the
coders behind it are not held up for nothing.

Under fifo and edf the keys define one global order, so the waiting coder with the
smallest key is at the top of both of its queues. It always reserves its dongles and is
served as soon as they are free. Every waiter therefore eventually becomes that coder,
and nobody starves.

### Dongle cooldown

`dongles_release()` stores `free_at = now + dongle_cooldown` on both dongles. A waiting
coder may take them only when `now >= free_at`. Because nobody signals the moment a
cooldown expires, waiters use `pthread_cond_timedwait` with a 1 ms timeout and re-check
the condition.

### Precise burnout detection

A dedicated **monitor thread** scans every coder every **1 ms**, comparing
`now - last_compile` with `time_to_burnout`. When a coder burns out, the monitor
**locks the print mutex, sets the stop flag, prints `burned out`, and unlocks**. The
announcement is therefore at most ~1–2 ms late (well within the 10 ms limit) and is
always the last line printed.

### Single coder

With one coder there is a single dongle (`first == second`), so the coder can never
compile. It logs `has taken a dongle` once and then waits until the monitor reports the
burnout.

### Clean termination

All waits are bounded: `sleep_ms()` checks the stop flag every 0.5 ms and dongle waits
time out every 1 ms. After a stop, every thread therefore exits within about a
millisecond and is joined before any resource is destroyed.

### Log serialization

Every state message goes through `log_state()`, which holds `print_lock` for the whole
`printf` and prints only while the simulation is running. When a coder gets its
dongles, `log_compile()` prints its two `has taken a dongle` lines and its `is compiling`
line in **one** critical section, so they always appear consecutively. Lines never
interleave, and nothing appears after `burned out`.

## Thread synchronization mechanisms

| Primitive | Protects | Used by |
|---|---|---|
| `pthread_mutex_t sim.table_lock` | the arbitration: checking and taking both dongles atomically, the fifo `ticket` | coders in `dongles_take` / `dongles_release` |
| `pthread_cond_t sim.table_cond` | waiting for dongles without busy-spinning | coders wait in `dongles_take`, `dongles_release` broadcasts |
| `pthread_mutex_t dongle.lock` (one per dongle) | each dongle's `taken`, `free_at` and heap, while they are modified | `join_queue`, `grab`, `give_back` |
| `pthread_mutex_t sim.state_lock` | `stop`, each coder's `last_compile` and `compiles` | coders write, monitor reads |
| `pthread_mutex_t sim.print_lock` | standard output | every log line and the burnout message |

**Waiting for dongles (mutex + condition variable).** In `dongles_take`, a coder locks
`table_lock`, pushes its request into both dongles' heaps, and then loops:

```c
while (sim_running(sim) && !can_go(c))   // both free + cooled + not reserved by others
    pthread_cond_timedwait(&sim->table_cond, &sim->table_lock, &one_ms_later);
```

`pthread_cond_timedwait` releases `table_lock` while sleeping and reacquires it on
wake-up. The condition is always re-checked in a `while` loop, which covers spurious
wake-ups, timeouts and broadcasts. When the check succeeds, the coder removes its
request from both heaps and marks both dongles as taken before releasing `table_lock`,
so no other coder can take them in between. `dongles_release` updates both dongles under
the same lock and calls `pthread_cond_broadcast` to wake the waiters.

**Coder ↔ monitor communication.** The coder writes `last_compile` (when it starts
compiling) and `compiles` (after compiling) under `state_lock`. The monitor reads the
same fields under the same lock, so it never sees a half-written value. Without the
lock, `compiles++` (a read-modify-write) racing with the monitor's read would be a data
race.

**Stopping.** The stop flag is only read or written through `sim_running()` and
`sim_stop()`, both of which take `state_lock`. Every loop (coder routine, dongle wait,
`sleep_ms`) polls `sim_running()`.

**Lock ordering.** Nested locks always follow the same order:
`table_lock → dongle.lock`, `table_lock → state_lock` (EDF key and `sim_running` inside
`dongles_take`) and `print_lock → state_lock` (logging and the burnout announcement).
`dongle.lock` and `state_lock` are always innermost, and only one dongle lock is held at a
time, so the locks themselves cannot deadlock.

The implementation was checked with `-fsanitize=thread` (no data races),
`-fsanitize=address,undefined` and macOS `leaks` (no leaks).

## Resources

- *The Dining Philosophers Problem*: E. W. Dijkstra, *Hierarchical ordering of
  sequential processes* (1971).
- Coffman, Elphick and Shoshani, *System Deadlocks* (1971): the four necessary
  conditions for deadlock.
- POSIX Threads Programming, Lawrence Livermore National Laboratory:
  <https://hpc-tutorials.llnl.gov/posix/>
- `man 3 pthread_mutex_lock`, `man 3 pthread_cond_timedwait`, `man 2 gettimeofday`.
- Binary heaps / priority queues: Cormen et al., *Introduction to Algorithms*, chapter 6.
- Earliest Deadline First scheduling: Liu and Layland, *Scheduling Algorithms for
  Multiprogramming in a Hard-Real-Time Environment* (1973).

### Use of AI

An AI assistant (Claude) was used as a **tutor and reviewer**:

- **Concepts**: explaining threads, mutexes, condition variables, data races, deadlock
  and Coffman's conditions, and binary heaps, with analogies and small examples.
- **Architecture**: proposing the file layout and the structures before any code was
  written.
- **Code review**: the C logic (parsing, timing, heap, state, init/cleanup, dongles,
  coders, monitor) was written by me. The AI compiled and tested it after each step,
  pointed out errors and explained them, and I fixed them myself.
- **Testing**: writing throw-away test harnesses (outside the repository) and running
  them with ThreadSanitizer, AddressSanitizer and `leaks`, plus a log checker that runs
  every benchmark case of the evaluation sheet several times. This is how the dongle
  "chains" were found.
- **Written by the AI at my request**: the `Makefile`, the final `main.c` (thread
  creation and joining), Norm formatting fixes, and this README. After the evaluation
  sheet's benchmark cases were run, the AI also redesigned the dongle acquisition: taking
  both dongles atomically (`dongle.c`, `dongle_utils.c`), `heap_remove`, `log_compile`
  and the single-coder case. This fixed the start-up chains, the burnouts in the cooldown
  benchmark, and the "two takes right before compiling" log rule. All of it was reviewed
  and explained line by line so that I can justify every part.
