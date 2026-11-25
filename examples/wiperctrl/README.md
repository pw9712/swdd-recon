# Example: WiperCtrl

A front-wiper application SWC written in the style of a long-lived AUTOSAR 4.x project:
it works, but it carries ten years of history (migration from AR 3.2, features bolted on,
comments that drifted, an ARXML that no longer matches the code).

It is the reference input for swdd-recon: the tool should reconstruct its detailed design
and detect the planted inconsistencies.

## Contents

| Path | What | Tool input? |
|---|---|---|
| `src/WiperCtrl.c` | Runnables (Init, 10 ms main, 100 ms background), RTE I/O, DEM/NvM | yes |
| `src/WiperCtrl_Sm.c` | Mode state machine (OFF, INTERVAL, AUTO, LOW, HIGH, WASH, PARKING, BLOCKED, SERVICE) | yes |
| `src/WiperCtrl_Motor.c` | Motor command, blocking detection, cool-down/retry/lock | yes |
| `src/WiperCtrl_Wash.c` | Washer pump and after-wipe | yes |
| `src/WiperCtrl_Rain.c` | Rain sensor evaluation for AUTO, hysteresis | yes |
| `src/*.h` | Public, private and configuration headers | yes |
| `rte/` | `Rte_WiperCtrl.h`, `Rte_WiperCtrl_Type.h`: stand-in for generated RTE headers | yes |
| `platform/` | Host stubs: `Std_Types.h`, `Platform_Types.h`, `Compiler.h`, `WiperCtrl_MemMap.h`, `Det.h` | yes |
| `arxml/WiperCtrl.arxml` | SWC description: types, interfaces, ports, runnables, events, PIM, calibration | yes |
| `test/` | Host RTE stub and plant-model simulation | no |
| `ground_truth/SPEC.md` | Intended behaviour | **no** |
| `ground_truth/EXPECTED_FINDINGS.md` | Answer key: 17 findings + 3 traps | **no** |

## Build and run

```sh
make test     # build with stubs and run the scenarios
make clean
```

Requires a C99 compiler and make. Tested with GCC 13 on Linux.

## Notes

- `Compiler.h` macros (`FUNC(void, WiperCtrl_CODE)`) and the MemMap pattern (header included
  repeatedly, no include guard) are kept on purpose. They are the first obstacle for a parser.
- The ARXML is hand-written and schema-plausible, but not validated against the official XSD.
  All internal references resolve.
- Variant `WIPERCTRL_SPEED_DEPENDENT` is `STD_OFF`. Analysis must honour the configuration
  (see F-02 in the answer key).
