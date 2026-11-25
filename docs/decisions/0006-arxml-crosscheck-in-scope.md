# 0006 – Code vs. ARXML cross-check is in scope

Status: accepted

## Decision

The tool compares the code with the SWC description: undeclared or unused ports, missing access and
call points, runnable timing vs. assumptions in the code, inter-runnable data without IRV or exclusive areas.

## Consequences

- Needs a call graph mapped to runnables: which function executes in which runnable (F-03, F-05, F-06).
- Must be configuration-aware, honouring variant switches (F-02).
