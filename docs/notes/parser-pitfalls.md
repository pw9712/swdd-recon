# Parser pitfalls

Learned while building and verifying the WiperCtrl example.

1. **Compiler Abstraction and MemMap break naive parsing.** `FUNC(void, WiperCtrl_CODE)`, `P2VAR(...)`
   and the MemMap pattern (header included repeatedly, no include guard) confuse a parser on raw sources.
   → Preprocess with the stub headers using `gcc -E -C`, which keeps comments (the LLM and several
   findings need them). Map preprocessed lines back to the original file and line via the `#` line markers.
2. **Analysis must be configuration-aware.** A textual scan reports `VehicleSpeed` as used, but its only
   read is inside `#if (WIPERCTRL_SPEED_DEPENDENT == STD_ON)` and the variant is `STD_OFF` (F-02).
3. **State machine extraction** must handle:
   - assignments with integer literals (`State = 6u;`), mapped back to enum constants;
   - transitions hidden in helper functions called from the `switch`.
4. **Some findings need a call graph mapped to runnables** (which function executes in which runnable):
   F-03 (server call point), F-05 (timing), F-06 (globals shared between runnables).
5. **Not every suspicious construct is a bug.** The apparent division by zero in `WiperCtrl_Rain.c` is
   unreachable (all input combinations checked). It is a false-positive trap: don't report it.

## Behaviour visible in `make test`

- After-wipe lasts 1.5 s instead of 3 s (F-05).
- Holding the washer button bypasses the pump run-time limit.
- With the hood open, the motor starts during a retry after blocking (F-12, found during verification, not planned).
