# Register Construction Chain

Implementing the full register hierarchy from Chapter 4 of
*The Art of Multiprocessor Programming*.

Each construction builds a stronger register from weaker ones.

## Chain

| # | From                  | To                    | File                       | Key idea                        |
|---|-----------------------|-----------------------|----------------------------|---------------------------------|
| 1 | SRSW safe Boolean     | MRSW safe Boolean     | `mrsw_safe_bool.h`        | One copy per reader             |
| 2 | MRSW safe Boolean     | MRSW regular Boolean  | `mrsw_regular_bool.h`     | Skip redundant writes           |
| 3 | MRSW regular Boolean  | MRSW regular M-valued | `mrsw_regular_mval.h`     | Unary bit encoding              |
| 4 | MRSW regular          | SRSW atomic           | `srsw_atomic.h`           | Timestamps, reader remembers    |
| 5 | SRSW atomic           | MRSW atomic           | `mrsw_atomic.h`           | Readers help readers (n×n table)|
| 6 | MRSW atomic           | MRMW atomic           | `mrmw_atomic.h`           | Writer timestamps               |

## Shared

- `register.h` — common types, thread ID, StampedValue

## Note on composition
 
Each step conceptually wraps the register type from the previous step (e.g.
step 2 should use a `MRSWSafeBoolRegister` from step 1 as its underlying
storage, step 3's bit array should be `MRSWRegularBoolRegister` instances,
etc.). In this implementation, we use plain C types (`bool`, `StampedValue`)
instead of actual struct composition. This matches the book's approach — it
uses naming conventions (`s_` for safe, `r_` for regular, `a_` for atomic)
rather than nesting the actual types. The algorithms and their correctness
arguments are the same either way.
