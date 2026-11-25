# WiperCtrl – planted findings (ground truth)

> **Do not feed this folder to the tool.** This is the answer key used to score
> the tool's output (recall / precision of findings).

The example is deliberately "legacy": it works (see `make test`) but contains
inconsistencies a reverse-engineering tool should detect. *Detection* says what
is expected to find it:

- **static**: deterministic, parser + ARXML model
- **static+cfg**: deterministic, but only if preprocessing honours the configuration (`WiperCtrl_Cfg.h`)
- **AI**: needs semantic reasoning (comments, units, behaviour across files)

## A. Code vs. ARXML (architecture consistency)

| ID | Finding | Location | Detection |
|---|---|---|---|
| F-01 | **Undeclared port.** Code reads `HoodStatus.Open` (`Rte_Read_HoodStatus_Open`), the SWC description has no `HoodStatus` port. | `WiperCtrl.c` / `WiperCtrl_ReadInputs` | static |
| F-02 | **Port declared but effectively unused.** `VehicleSpeed` is declared with a `DATA-READ-ACCESS` in `RE_WiperCtrl_MainFunction`, but the only access is inside `#if (WIPERCTRL_SPEED_DEPENDENT == STD_ON)` and the variant is `STD_OFF`. A purely textual scan reports it as used. | `WiperCtrl_Sm.c` / case `WIPER_ST_HIGH` | static+cfg |
| F-03 | **Server call without declared call point.** `RE_WiperCtrl_Init` calls `Rte_Call_NvMService_WiperCfg_GetErrorStatus`; ARXML declares that server call point only for `RE_WiperCtrl_Background`. | `WiperCtrl.c` / `RE_WiperCtrl_Init` | static (needs call graph: function → runnable) |
| F-04 | **Scaling mismatch.** ARXML `CM_MotorCurrent` is 1 mA/bit; code assumes 0.1 A/bit (`WIPERCTRL_BLOCK_CURRENT 120u /* 12.0 A */`). With the ARXML scaling the threshold is 120 mA and blocking detection would trigger during normal operation. | `WiperCtrl_Cfg.h`, `WiperCtrl_Motor.c` | AI (unit in comment vs. compu method) |
| F-05 | **Timing assumption mismatch.** `WIPERCTRL_WASH_AFTERWIPE_TICKS 150u /* 3 s @ 20 ms task */`, but the wash logic runs in the 10 ms runnable (ARXML `EV_Main_10ms`, PERIOD 0.01) → after-wipe lasts 1.5 s; spec requires 3 s. Observable in `make test` (S5). | `WiperCtrl_Cfg.h`, `WiperCtrl_Wash.c` | AI (comment + call graph + ARXML timing) |
| F-06 | **Inter-runnable data without IRV/exclusive area.** `WiperCtrl_MotorBlocked` and `WiperCtrl_In` are written in `RE_WiperCtrl_MainFunction` (10 ms) and read in `RE_WiperCtrl_Background` (100 ms) as plain globals; ARXML declares no `INTER-RUNNABLE-VARIABLES` or `EXCLUSIVE-AREAS`. Data consistency risk if the runnables are mapped to different tasks. | `WiperCtrl.c` | static (globals R/W per runnable) |

## B. Behaviour / design

| ID | Finding | Location | Detection |
|---|---|---|---|
| F-07 | **Stale comment.** `WIPERCTRL_BLOCK_DEBOUNCE 30u /* 500 ms */` → 300 ms at 10 ms (spec: 300 ms, so the comment is wrong, not the code). | `WiperCtrl_Cfg.h` | AI |
| F-08 | **Unreachable state.** `WIPER_ST_SERVICE` is handled in the state switch but never assigned; it also has no exit transition. | `WiperCtrl_Sm.c` | static (FSM extraction) |
| F-09 | **Magic numbers for states.** Transitions written as `WiperCtrl_State = 6u; /* parking */` and `7u; /* blocked */` instead of enum names. FSM extraction must map literals to enum values. | `WiperCtrl_Sm.c` | static |
| F-10 | **Ignored RTE return codes.** All `Rte_Read_*` except `RainSensor_Intensity` are cast to `(void)`. A stalk-switch signal timeout keeps the last value, so wipers may keep running. | `WiperCtrl.c` / `WiperCtrl_ReadInputs` | static (detect) + AI (consequence) |
| F-11 | **Wash pump max-on time can be bypassed.** Holding the button > 10 s stops the pump, runs the after-wipe, then IDLE sees `washReq == TRUE` and restarts the pump. Spec: no restart before release. Observable in `make test` (S6). | `WiperCtrl_Wash.c` | AI |
| F-12 | **Hood open does not stop a blocking retry (safety).** On hood open, the state machine requests STOP, but `WiperCtrl_Motor_Cyclic` in state `RETRY` drives LOW regardless of the request → motor runs up to 300 ms per retry with the hood open (pinch protection). | `WiperCtrl_Sm.c` + `WiperCtrl_Motor.c` | AI (cross-file sequence) |

## C. Coding rules / architecture smells

| ID | Finding | Location | Detection |
|---|---|---|---|
| F-13 | `switch` without `default` (MISRA C:2012 Rule 16.4). | `WiperCtrl_Wash.c` / `WiperCtrl_Wash_Cyclic` | static |
| F-14 | Multiple exit points (MISRA C:2012 Rule 15.5, advisory). | `RE_WiperCtrl_MainFunction`, `WiperCtrl_Sm_Cyclic`, `WiperCtrl_Wash_Cyclic` | static |
| F-15 | Application SWC calls `Det_ReportError` directly instead of going through an RTE service port. | `WiperCtrl.c` | static |
| F-16 | Dead code: SOP1 algorithm under `#if 0`. | `WiperCtrl_Rain.c` | static |
| F-17 | Shared globals across translation units (`WiperCtrl_In`, `WiperCtrl_State`, …) instead of access functions. Makes the dynamic view harder to derive. | `WiperCtrl_Priv.h` | static |

## D. Traps (must NOT be reported as defects)

| ID | Looks like | Why it is not a defect | Expected tool behaviour |
|---|---|---|---|
| T-01 | Division by zero / unsigned wrap in `(rel * 700u) / (50u - thr)` when calibration `RainThreshold >= 50`. | The INTERVAL branch is unreachable for `thr >= 50` (verified exhaustively for all thresholds, intensities, levels and previous demands). | At most "fragile, add a guard"; reporting it as a crash is a false positive. |
| T-02 | `WiperCtrl_IntTimer = 0xFFFFu` looks like an overflow. | Deliberate marker: "first wipe immediately". | Explain the intent. |
| T-03 | Motor command is written only in `WiperCtrl_Motor.c`, not in `WiperCtrl_WriteOutputs`. | Intentional, comment says so. | Sequence diagram must show the write from `WiperCtrl_Motor_Cyclic`. |

## Scoring idea

- Recall per category (A/B/C) and overall.
- Precision: reported findings that are not in this list and not real → false positives (T-xx count double).
- Each reported finding must cite file + function (+ line); findings without evidence count as not found.
