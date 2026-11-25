# WiperCtrl – intended behaviour (ground truth)

> **Do not feed this folder to the tool.** It describes what the SWC was *meant*
> to do. The tool only sees `src/`, `rte/`, `platform/` and `arxml/`; this file
> is used afterwards to score the reconstructed design.

## Purpose

Application SWC controlling the front windscreen wiper of a passenger car:
manual modes (interval, low, high), rain-sensor controlled AUTO mode, wash/wipe
function, motor blocking protection and service counters in NvM.

## Runnables

| Runnable | Trigger | Responsibility |
|---|---|---|
| `RE_WiperCtrl_Init` | Init event | Initialise internal state, check NvM block, apply defaults |
| `RE_WiperCtrl_MainFunction` | Timing event, 10 ms | Read inputs, state machine, motor control, wash, outputs |
| `RE_WiperCtrl_Background` | Timing event, 100 ms | Diagnostics (DEM), NvM write on power-down |

## Interfaces

| Port | Dir | Data / operation | Notes |
|---|---|---|---|
| StalkSwitch | in | WiperPos (OFF/INT/LOW/HIGH), IntLevel (1..4), WashReq | Steering column stalk |
| RainSensor | in | Intensity (0..100 %), Status (OK/INIT/FAULT) | LIN rain/light sensor |
| PowerMode | in | Mode (OFF/ACC/RUN/CRANK) | Wipers active only in RUN |
| WiperMotorFb | in | ParkPos (bool), Current (mA, 1 mA/bit) | From IoHwAb |
| VehicleSpeed | in | Speed (0.1 km/h/bit) | Speed-dependent HIGH->LOW (variant, currently off) |
| HoodStatus | in | Open (bool) | Added in 2019, see history |
| WiperMotorCmd | out | Speed (STOP/LOW/HIGH) | To IoHwAb |
| WiperStatus | out | Mode (OFF/INTERVAL/AUTO/LOW/HIGH/WASH/FAULT) | To cluster via COM |
| WasherPump | client | SetState(On) | IoHwAb |
| Event_MotorBlocked | client | SetEventStatus | DEM |
| Event_RainSensorFault | client | SetEventStatus | DEM |
| NvMService_WiperCfg | client | GetErrorStatus, WriteBlock | NvM, mirror in PIM `NvMirror` |
| RainThreshold | calibration | uint8, default 10 % | Shared parameter |

## Modes and transitions

Priority (highest first): hood open > motor blocked > power mode not RUN > wash > stalk position.

- **OFF** – motor stopped, wiper in park position.
- **INTERVAL** – stalk INT with rain sensor not OK. One wipe immediately on entry,
  then one wipe every *delay* after returning to park. Delay by IntLevel:
  1 = 12 s, 2 = 7 s, 3 = 4 s, 4 = 2 s.
- **AUTO** – stalk INT with rain sensor OK. IntLevel acts as sensitivity
  (+5 % per level above 1). Effective intensity vs. calibration `RainThreshold`:
  below threshold → no wiping; up to 50 % → interval wiping with delay linearly
  8 s → 1 s; 50–80 % → LOW; above 80 % → HIGH. Hysteresis 5 % on each boundary
  when stepping down.
- **LOW / HIGH** – continuous wiping at the selected speed.
- **WASH** – entered while the wash button is pressed (from OFF, INTERVAL, AUTO,
  LOW, HIGH, PARKING). Pump on while pressed, max 10 s. Wipers run LOW (HIGH if
  stalk is HIGH). After release: after-wipe for **3 s**, then back to the mode
  selected by the stalk. Holding the button longer than 10 s must **not**
  restart the pump until the button has been released.
- **PARKING** – leaving any wiping mode (stalk OFF or power mode not RUN): keep
  LOW until the park position is reached, then OFF. Never stop mid-glass, except
  hood open.
- **BLOCKED** – motor current above 12 A for 300 ms: stop motor, 5 s cool-down,
  retry at LOW; at most 3 retries. A retry succeeds when the park position is
  reached. After the last failed retry the motor stays locked until the driver
  moves the stalk from OFF to an active position. DEM event `MotorBlocked`
  FAILED while blocked, PASSED after recovery.
- **SERVICE** – planned (blade change position via diagnostic routine), not
  implemented.

Hood open: motor stops immediately (pinch protection), wash disabled.

## Diagnostics and NvM

- Rain sensor status FAULT for 2 s → DEM `RainSensorFault` FAILED; status OK → PASSED.
  AUTO mode falls back to INTERVAL immediately on FAULT.
- NvM block (PIM mirror `NvMirror`): `WashCycleCnt` (completed wash cycles,
  saturating 16 bit), `BlockEventCnt` (blocking events, saturating 8 bit).
  Mirror filled by NvM_ReadAll; defaults (0) if the block is not OK at init.
  Written once on the transition from RUN to any other power mode, unless a
  request is pending.
