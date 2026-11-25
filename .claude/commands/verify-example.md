---
description: Build the WiperCtrl example with strict warnings and run the host simulation
---
Run `make -C examples/wiperctrl clean test` and report:
- whether it compiled without warnings,
- how many checks failed (the exit code) and in which scenarios.

Expected: 0 failed checks. Do not modify files under `examples/wiperctrl/` to make it pass;
the planted findings (see `ground_truth/`) are intentional.
