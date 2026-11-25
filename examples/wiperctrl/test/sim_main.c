/**
 * \file   sim_main.c
 * \brief  Host simulation of WiperCtrl with a simple wiper motor plant model.
 *
 * Purpose: prove the example SWC builds and behaves plausibly, and make some of
 * the planted findings (ground_truth/EXPECTED_FINDINGS.md) observable.
 * Time base: 1 tick = 10 ms. Background runnable every 10th tick.
 *
 * Exit code = number of failed checks (0 = pass), usable in CI.
 */
#include <stdio.h>
#include "Rte_WiperCtrl.h"
#include "Rte_Stub.h"
#include "WiperCtrl_Priv.h"     /* white-box: state for logging */

/* ---- plant model --------------------------------------------------------- */
#define PLANT_CYCLE         1000u   /* one wipe = 1000 phase units */
#define PLANT_PARK_WIDTH    60u     /* park switch closed for phase < 60 */
#define PLANT_STEP_LOW      10u     /* 1.0 s per wipe */
#define PLANT_STEP_HIGH     15u     /* ~0.67 s per wipe */
#define PLANT_I_RUN         35u     /* 3.5 A in 0.1 A/bit, as the SWC expects (see F-04) */
#define PLANT_I_STALL       150u    /* 15 A */

static unsigned long  g_tick;
static unsigned       g_phase;
static int            g_blocked;
static unsigned       g_wipes;          /* completed wipes (park rising edges) */
static unsigned       g_starts;         /* motor STOP -> running transitions */
static unsigned long  g_runTicks;       /* ticks with motor running */
static MotorSpeedCmd_T g_lastCmd = MOTORSPEED_STOP;
static WiperCtrl_StateType g_lastState = WIPER_ST_OFF;
static int            g_fail;

static const char *const k_state[] = {
    "OFF", "INTERVAL", "LOW", "HIGH", "AUTO", "WASH", "PARKING", "BLOCKED", "SERVICE"
};
static const char *const k_motor[] = { "STOP", "LOW", "HIGH" };

#define T()  ((double)g_tick / 100.0)

static void check(int cond, const char *msg)
{
    printf("  %s %s\n", cond ? "ok  " : "FAIL", msg);
    if (!cond) { g_fail++; }
}

static void plant_step(void)
{
    unsigned inc = 0u;
    int park;

    if (RteStub_WiperMotorCmd_Speed == MOTORSPEED_LOW)  { inc = PLANT_STEP_LOW; }
    if (RteStub_WiperMotorCmd_Speed == MOTORSPEED_HIGH) { inc = PLANT_STEP_HIGH; }

    if (g_blocked)
    {
        RteStub_WiperMotorFb_Current = (inc != 0u) ? PLANT_I_STALL : 0u;
        inc = 0u;
    }
    else
    {
        RteStub_WiperMotorFb_Current = (inc != 0u) ? PLANT_I_RUN : 0u;
    }

    g_phase += inc;
    if (g_phase >= PLANT_CYCLE) { g_phase -= PLANT_CYCLE; }

    park = (g_phase < PLANT_PARK_WIDTH);
    if (park && !RteStub_WiperMotorFb_ParkPos) { g_wipes++; }
    RteStub_WiperMotorFb_ParkPos = park ? TRUE : FALSE;
}

static void run(double seconds)
{
    unsigned long n = (unsigned long)(seconds * 100.0 + 0.5);
    while (n-- > 0u)
    {
        plant_step();
        RE_WiperCtrl_MainFunction();
        if ((g_tick % 10u) == 0u) { RE_WiperCtrl_Background(); }
        g_tick++;

        if ((g_lastCmd == MOTORSPEED_STOP) && (RteStub_WiperMotorCmd_Speed != MOTORSPEED_STOP)) { g_starts++; }
        if (RteStub_WiperMotorCmd_Speed != MOTORSPEED_STOP) { g_runTicks++; }
        g_lastCmd = RteStub_WiperMotorCmd_Speed;

        if (WiperCtrl_State != g_lastState)
        {
            printf("  [%7.2f s] %-8s -> %-8s motor=%s park=%d\n", T(),
                   k_state[g_lastState], k_state[WiperCtrl_State],
                   k_motor[RteStub_WiperMotorCmd_Speed], RteStub_WiperMotorFb_ParkPos);
            g_lastState = WiperCtrl_State;
        }
    }
}

static void section(const char *name) { printf("\n== %s\n", name); }

int main(void)
{
    unsigned w0, s0;
    unsigned long r0, t0;
    unsigned wash0;

    RteStub_Reset();
    RE_WiperCtrl_Init();

    /* ------------------------------------------------------------------ */
    section("S1 power up, stalk OFF");
    RteStub_PowerMode_Mode = POWERMODE_RUN;
    run(0.5);
    check(WiperCtrl_State == WIPER_ST_OFF, "state OFF");
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_STOP, "motor STOP");

    /* ------------------------------------------------------------------ */
    section("S2 stalk LOW, then OFF -> parking");
    w0 = g_wipes;
    RteStub_StalkSwitch_WiperPos = WIPERPOS_LOW;
    run(3.05);
    check(WiperCtrl_State == WIPER_ST_LOW, "state LOW");
    check(RteStub_WiperStatus_Mode == WIPERSTATUS_LOW, "status LOW reported");
    check((g_wipes - w0) >= 2u, ">= 2 wipes in 3 s");
    RteStub_StalkSwitch_WiperPos = WIPERPOS_OFF;
    run(1.2);
    check(WiperCtrl_State == WIPER_ST_OFF, "state OFF after parking");
    check(RteStub_WiperMotorFb_ParkPos == TRUE, "wiper in park position");

    /* ------------------------------------------------------------------ */
    section("S3 interval, rain sensor faulty, level 4 (2 s)");
    RteStub_RainSensor_Status    = RAINSENSOR_FAULT;
    RteStub_StalkSwitch_IntLevel = 4u;
    RteStub_StalkSwitch_WiperPos = WIPERPOS_INT;
    w0 = g_wipes;
    run(10.0);
    check(WiperCtrl_State == WIPER_ST_INTERVAL, "state INTERVAL");
    check(((g_wipes - w0) >= 3u) && ((g_wipes - w0) <= 4u), "3..4 wipes in 10 s (1 s wipe + 2 s pause)");
    check(RteStub_Dem_RainSensorFault == DEM_EVENT_STATUS_FAILED, "DEM RainSensorFault FAILED");

    /* ------------------------------------------------------------------ */
    section("S4 AUTO mode");
    RteStub_StalkSwitch_WiperPos = WIPERPOS_OFF;
    run(1.5);
    RteStub_RainSensor_Status    = RAINSENSOR_OK;
    RteStub_RainSensor_Intensity = 0u;
    RteStub_StalkSwitch_IntLevel = 1u;
    RteStub_StalkSwitch_WiperPos = WIPERPOS_INT;
    run(2.0);
    check(WiperCtrl_State == WIPER_ST_AUTO, "state AUTO");
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_STOP, "no rain -> motor STOP");
    check(RteStub_Dem_RainSensorFault == DEM_EVENT_STATUS_PASSED, "DEM RainSensorFault PASSED");
    RteStub_RainSensor_Intensity = 90u;
    run(1.0);
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_HIGH, "90 % -> HIGH");
    RteStub_RainSensor_Intensity = 60u;
    run(2.0);
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_LOW, "60 % -> LOW");
    RteStub_RainSensor_Intensity = 30u;
    w0 = g_wipes; r0 = g_runTicks; t0 = g_tick;
    run(12.0);
    printf("  info: 30 %% -> %u wipes in 12 s, duty %.0f %%\n", g_wipes - w0,
           100.0 * (double)(g_runTicks - r0) / (double)(g_tick - t0));
    check(((g_wipes - w0) >= 2u) && ((g_runTicks - r0) * 2u < (g_tick - t0)), "30 % -> interval wiping (duty < 50 %)");
    RteStub_RainSensor_Intensity = 0u;
    run(1.5);
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_STOP, "rain stops -> motor STOP");
    check(RteStub_WiperMotorFb_ParkPos == TRUE, "wiper parked");

    /* ------------------------------------------------------------------ */
    section("S5 wash / wipe from OFF");
    RteStub_StalkSwitch_WiperPos = WIPERPOS_OFF;
    run(1.5);
    wash0 = RteStub_NvMirror.WashCycleCnt;
    RteStub_StalkSwitch_WashReq = TRUE;
    run(2.0);
    check(RteStub_WasherPump_On == TRUE, "pump ON while button pressed");
    check(WiperCtrl_State == WIPER_ST_WASH, "state WASH");
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_LOW, "wipers LOW during wash");
    RteStub_StalkSwitch_WashReq = FALSE;
    t0 = g_tick;
    while ((WiperCtrl_State == WIPER_ST_WASH) && ((g_tick - t0) < 1000u)) { run(0.01); }
    check(RteStub_WasherPump_On == FALSE, "pump OFF after release");
    printf("  info: after-wipe lasted %.2f s (spec: 3.0 s) -> see F-05\n", (double)(g_tick - t0) / 100.0);
    run(1.5);
    check(WiperCtrl_State == WIPER_ST_OFF, "back to OFF, parked");
    check(RteStub_NvMirror.WashCycleCnt == (uint16)(wash0 + 1u), "wash cycle counter +1");

    /* ------------------------------------------------------------------ */
    section("S6 wash button held 15 s");
    s0 = RteStub_WasherPump_CallCnt;
    RteStub_StalkSwitch_WashReq = TRUE;
    run(15.0);
    RteStub_StalkSwitch_WashReq = FALSE;
    run(5.0);
    printf("  info: pump switched ON %u time(s) for one press (spec: 1) -> see F-11\n",
           (RteStub_WasherPump_CallCnt - s0 + 1u) / 2u);

    /* ------------------------------------------------------------------ */
    section("S7 motor blocking, 3 retries, lock, re-activation");
    RteStub_StalkSwitch_WiperPos = WIPERPOS_LOW;
    run(1.0);
    g_blocked = 1;
    s0 = g_starts;
    run(0.5);
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_STOP, "blocked -> motor STOP");
    check(WiperCtrl_State == WIPER_ST_BLOCKED, "state BLOCKED");
    run(1.0);
    check(RteStub_Dem_MotorBlocked == DEM_EVENT_STATUS_FAILED, "DEM MotorBlocked FAILED");
    run(25.0);
    check((g_starts - s0) == WIPERCTRL_BLOCK_MAX_RETRY, "exactly 3 retries");
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_STOP, "locked -> motor STOP");
    g_blocked = 0;
    run(2.0);
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_STOP, "still locked while stalk stays LOW");
    RteStub_StalkSwitch_WiperPos = WIPERPOS_OFF;
    run(0.2);
    RteStub_StalkSwitch_WiperPos = WIPERPOS_LOW;
    run(2.0);
    check(WiperCtrl_State == WIPER_ST_LOW, "stalk OFF->LOW re-activates: state LOW");
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_LOW, "motor LOW");
    check(RteStub_Dem_MotorBlocked == DEM_EVENT_STATUS_PASSED, "DEM MotorBlocked PASSED");
    check(RteStub_NvMirror.BlockEventCnt == 3u, "block event counter 2 -> 3");

    /* ------------------------------------------------------------------ */
    section("S8 hood opened while wiping");
    run(0.45);
    RteStub_HoodStatus_Open = TRUE;
    run(0.02);
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_STOP, "hood open -> immediate STOP");
    printf("  info: stopped mid-glass: %s\n", RteStub_WiperMotorFb_ParkPos ? "no" : "yes");
    run(1.0);
    RteStub_HoodStatus_Open = FALSE;
    run(1.0);
    check(WiperCtrl_State == WIPER_ST_LOW, "hood closed -> LOW again");

    /* ------------------------------------------------------------------ */
    section("S9 power mode OFF while wiping");
    run(0.4);
    RteStub_PowerMode_Mode = POWERMODE_OFF;
    run(1.5);
    check(WiperCtrl_State == WIPER_ST_OFF, "parked and OFF");
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_STOP, "motor STOP");
    check(RteStub_WiperMotorFb_ParkPos == TRUE, "wiper in park position");
    check(RteStub_NvM_WriteCnt == 1u, "NvM WriteBlock once");
    run(2.0);
    check(RteStub_WiperMotorCmd_Speed == MOTORSPEED_STOP, "stalk LOW ignored without RUN");

    printf("\n%s: %d failed check(s), %.1f s simulated, DET errors: %u\n",
           g_fail ? "FAILED" : "PASSED", g_fail, T(), RteStub_DetErrorCnt);
    return g_fail;
}
