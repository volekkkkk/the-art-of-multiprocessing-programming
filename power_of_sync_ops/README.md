# Chapter 5 — Task list

Practical work for *The Art of Multiprocessor Programming*, Chapter 5:
The Relative Power of Primitive Synchronization Operations.

## Layout

```
chapter5/
├── consensus.h           # generic Consensus interface
├── consensus_test.h      # N-thread trial harness (barrier-synced)
├── atomic_register.c     # Task 2
├── queue.c               # Task 3
├── multi_assign.c        # Task 4
├── rmw_swap.c            # Task 5
├── rmw_incr.c            # Task 6
└── exercises/
    ├── ex57_notes.md
    ├── ex50_notes.md
    └── ex75_notes.md
```

Naming follows the register-only convention from Chapter 4: file named
after the primitive, not the section.

## Backbone tasks

Each backbone task plugs a new protocol into the shared harness and
either passes for all N or fails at a predictable N. The failures are
as important as the passes — they empirically confirm the impossibility
theorems.

### Task 1 — Infrastructure

**Files:** `consensus.h`, `consensus_test.h`

**Contents of `consensus.h`:** a generic protocol interface following
Fig 5.6 — a `proposed[N]` array, a `propose(v)` that stores into it,
and an abstract `decide(v)` to be filled in by each protocol. Also
declare the shared per-thread ID mechanism.

**Contents of `consensus_test.h`:** a header-only harness that spawns
N pthreads, has each propose a distinct value (usually its thread ID or
`ID + 1000` to keep zero unambiguous), collects the N return values,
and asserts on each trial:

- **Consistency:** all N returned values are equal.
- **Validity:** the returned value equals one of the proposed inputs.

Trials are barrier-synced so threads actually race at each `decide()`
call. Run ~100k trials per test. Report per-trial pass/fail plus a
final summary.

**Success criteria:** harness compiles, runs, and (using a trivial
mocked protocol like "always return proposed[0]") reports 100% pass.

### Task 2 — Atomic register attempt

**File:** `atomic_register.c`

**Primitive:** only `atomic_load` / `atomic_store` on `_Atomic int`.

**Protocol:** the obvious naïve attempt. Each thread writes its value
to its own slot, reads the other's slot, and decides based on what it
sees. Try several variants — none of them will work.

**Expected result at N=2:** the harness detects consistency violations
in some fraction of trials.

**What this demonstrates:** Theorem 5.2.1 empirically. The registers
carry no ordering information, so both threads can simultaneously
conclude they went first (or last).

### Task 3 — FIFO queue

**File:** `queue.c`

**Primitive:** a bounded ring buffer with `_Atomic` head/tail indices,
preloaded with WIN then LOSE at init time. Only `deq()` is needed at
runtime — no runtime enqueues, so a very simple queue suffices.

**Protocol (§5.4):** `propose(v)`; `deq()`; if got WIN return
`proposed[self]`, else return `proposed[other]`.

**Expected results:**
- N=2: 100% pass.
- N=3: fails on some trials — two threads dequeue LOSE and disagree on
  whose value to return.

**What this demonstrates:** Theorem 5.4.1 — FIFO queues have consensus
number exactly 2.

### Task 4 — Multiple assignment

**File:** `multi_assign.c`

**Primitive:** an `(n, n(n+1)/2)`-assignment object simulated with a
single mutex around the multi-cell write. C11 doesn't give us
hardware-atomic multi-word writes, so this is a simulation of the
capability rather than a wait-free implementation. That's fine — the
point is to verify the protocol logic (pairwise-ordering read rule,
picking the earliest writer).

**Protocol (§5.5, Fig 5.13):** each thread atomically assigns its
value to `r_i` and to every `r_ij` it participates in. Then reads all
cells and applies the pairwise rule from Figure 5.13 to determine the
earliest writer.

**Expected result at any N:** 100% pass.

**What this demonstrates:** Theorem 5.5.2 — multiple assignment has
consensus number at least n. Also drives home the duality point from
the paragraph we discussed: multi-write atomicity is strictly stronger
than multi-read atomicity.

### Task 5 — getAndSet consensus

**File:** `rmw_swap.c`

**Primitive:** `atomic_exchange` on `_Atomic int`.

**Protocol (§5.6):** shared int initialized to `-1`. Each thread
proposes, then `atomic_exchange`s its own thread ID into the shared
int. Whoever gets `-1` back went first; returns `proposed[self]`.
The others return `proposed[first_id]`.

**Expected results:**
- N=2: 100% pass.
- N=3: fails on some trials.

**What this demonstrates:** the first data point for the §5.7 Common2
result — nontrivial RMW gives you exactly 2-thread consensus and no
more.

### Task 6 — getAndIncrement consensus

**File:** `rmw_incr.c`

**Primitive:** `atomic_fetch_add` on `_Atomic int`.

**Protocol:** shared counter initialized to 0. Each thread proposes,
then `atomic_fetch_add(&counter, 1)`. Whoever gets 0 back went first;
returns own input. The rest need to somehow figure out the winner —
worth thinking about how, because the answer is instructive (there's
no clean way with just fetch-add, which is part of the point).

**Expected results:**
- N=2: passes.
- N=3: fails.

**What this demonstrates:** the second data point for Common2 — a
completely different primitive hits the same ceiling. When we read
§5.7 the proof will explain *why*, but you'll already have felt it.

## Exercises (write-ups, not code)

Save under `chapter5/exercises/` as short markdown notes. Aim for one
page each — the value is in writing the argument cleanly, not in
volume.

### Exercise 57 — Post-dequeue announce

**Question:** In the §5.4 queue consensus, what breaks if `propose()`
happens *after* `deq()` instead of before?

**Deliverable:** a concrete interleaving that violates either
consistency or validity, plus a one-line statement of the invariant
the original protocol depends on.

### Exercise 50 — Impossibility reduction

**Question:** Show that if binary consensus using atomic registers is
impossible for two threads, then it is also impossible for any n > 2.

**Deliverable:** proof by reduction — assume an n-thread protocol
exists, construct a 2-thread one from it, contradict Theorem 5.2.1.

Reductions are the workhorse of the whole chapter; worth practicing on
this small clean case before we hit harder ones.

### Exercise 75 — Obstruction-free consensus with registers

**Question:** Give an obstruction-free consensus protocol using only
atomic registers. Explain where the critical-state impossibility proof
breaks when we drop the wait-free requirement and allow an oracle to
halt threads so others can run in isolation.

**Deliverable:** protocol sketch (pseudocode is fine, or a small
`obstruction_free_consensus.c` if you want to run it — but it won't
plug into the harness cleanly since the harness assumes wait-freedom).
Plus a paragraph on why the impossibility proof relies on wait-freedom
specifically.

This one is a genuine puzzle. The answer illuminates what the
impossibility theorem is really saying, and sets up why the universal
construction in Chapter 6 needs stronger primitives.

## Suggested session plan

| Session | Content                        |
|---------|--------------------------------|
| 1       | Task 1 + Task 2                |
| 2       | Task 3 + Exercise 57           |
| 3       | Task 4                         |
| 4       | Tasks 5 and 6                  |
| 5       | Exercises 50 and 75            |

After Session 5 we're at the end of §5.7 conceptually — everything
from atomic registers through Common2 covered both theoretically and
empirically. Then §5.8 (compareAndSet) and its universal consensus
implementation is the next arc, and it belongs in Chapter 6 alongside
the universal construction anyway.
