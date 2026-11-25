/*******************************************************************************
 *  Project     : BCM Front Wiper
 *  Component   : WiperCtrl (Application SWC)
 *  File        : WiperCtrl_Sm.c
 *  Description : Wiper mode state machine.
 *
 *  History
 *  Date        Author  Description
 *  ----------  ------  --------------------------------------------------------
 *  2014-03-12  JKO     Initial version
 *  2015-06-30  JKO     Parking state, never stop mid-glass
 *  2017-08-21  MZI     AUTO state
 *  2019-05-14  PWA     Hood open -> immediate stop
 *  2020-09-08  ASZ     Service position prepared (DIAG-212)
 ******************************************************************************/

#include "WiperCtrl_Priv.h"

/* single wipe phases (interval / auto interval) */
#define SW_WAIT     0u      /* parked, counting delay */
#define SW_START    1u      /* motor on, waiting until park switch opens */
#define SW_RUN      2u      /* wiping, waiting for park position */

/* =============================================================================
 * Data
 * ========================================================================== */
#define WiperCtrl_START_SEC_CONST_16
#include "WiperCtrl_MemMap.h"

/* interval delay [10 ms], index = IntLevel (1..4), index 0 unused */
STATIC CONST(uint16, WiperCtrl_CONST) WiperCtrl_IntDelayTbl[5] = { 0u, 1200u, 700u, 400u, 200u };

#define WiperCtrl_STOP_SEC_CONST_16
#include "WiperCtrl_MemMap.h"

#define WiperCtrl_START_SEC_VAR_CLEARED_16
#include "WiperCtrl_MemMap.h"

STATIC VAR(uint16, WiperCtrl_VAR) WiperCtrl_IntTimer;

#define WiperCtrl_STOP_SEC_VAR_CLEARED_16
#include "WiperCtrl_MemMap.h"

#define WiperCtrl_START_SEC_VAR_CLEARED_8
#include "WiperCtrl_MemMap.h"

STATIC VAR(uint8, WiperCtrl_VAR) WiperCtrl_SwPhase;

#define WiperCtrl_STOP_SEC_VAR_CLEARED_8
#include "WiperCtrl_MemMap.h"

/* =============================================================================
 * Code
 * ========================================================================== */
#define WiperCtrl_START_SEC_CODE
#include "WiperCtrl_MemMap.h"

STATIC FUNC(void, WiperCtrl_CODE) WiperCtrl_Sm_SelectFromStalk(void);
STATIC FUNC(MotorSpeedCmd_T, WiperCtrl_CODE) WiperCtrl_Sm_SingleWipe(uint16 period);

FUNC(void, WiperCtrl_CODE) WiperCtrl_Sm_Init(void)
{
    WiperCtrl_State    = WIPER_ST_OFF;
    WiperCtrl_IntTimer = 0u;
    WiperCtrl_SwPhase  = SW_WAIT;
}

/*------------------------------------------------------------------------------
 * WiperCtrl_Sm_Cyclic - called from RE_WiperCtrl_MainFunction (10 ms)
 * evaluates WiperCtrl_In, sets WiperCtrl_State and WiperCtrl_MotorReq
 *----------------------------------------------------------------------------*/
FUNC(void, WiperCtrl_CODE) WiperCtrl_Sm_Cyclic(void)
{
    boolean enable;
    boolean washActive;
    uint8   rainDemand;
    uint8   lvl;

    /* hood open: stop immediately, no parking (pinch protection) */
    if (WiperCtrl_In.HoodOpen == TRUE)
    {
        WiperCtrl_State    = WIPER_ST_OFF;
        WiperCtrl_MotorReq = MOTORSPEED_STOP;
        (void)WiperCtrl_Wash_Cyclic(FALSE, FALSE);
        return;
    }

    if (WiperCtrl_MotorBlocked == TRUE)
    {
        WiperCtrl_State = 7u;   /* blocked */
    }

    enable = (WiperCtrl_In.PowerMode == POWERMODE_RUN) ? TRUE : FALSE;

    washActive = WiperCtrl_Wash_Cyclic(WiperCtrl_In.WashReq,
                                       ((enable == TRUE) && (WiperCtrl_State != WIPER_ST_BLOCKED)) ? TRUE : FALSE);

    switch (WiperCtrl_State)
    {
        case WIPER_ST_OFF:
            WiperCtrl_MotorReq = MOTORSPEED_STOP;
            if (enable == TRUE)
            {
                if (washActive == TRUE)
                {
                    WiperCtrl_State = WIPER_ST_WASH;
                }
                else
                {
                    WiperCtrl_Sm_SelectFromStalk();
                }
            }
            break;

        case WIPER_ST_INTERVAL:
            if (enable == FALSE)
            {
                WiperCtrl_State = 6u;   /* parking */
            }
            else if (washActive == TRUE)
            {
                WiperCtrl_State = WIPER_ST_WASH;
            }
            else if ((WiperCtrl_In.WiperPos != WIPERPOS_INT) || (WiperCtrl_In.RainStatus == RAINSENSOR_OK))
            {
                WiperCtrl_Sm_SelectFromStalk();
            }
            else
            {
                lvl = WiperCtrl_In.IntLevel;
                if (lvl < 1u) { lvl = 1u; }
                if (lvl > 4u) { lvl = 4u; }
                WiperCtrl_MotorReq = WiperCtrl_Sm_SingleWipe(WiperCtrl_IntDelayTbl[lvl]);
            }
            break;

        case WIPER_ST_AUTO:
            if (enable == FALSE)
            {
                WiperCtrl_State = 6u;   /* parking */
            }
            else if (washActive == TRUE)
            {
                WiperCtrl_State = WIPER_ST_WASH;
            }
            else if ((WiperCtrl_In.WiperPos != WIPERPOS_INT) || (WiperCtrl_In.RainStatus != RAINSENSOR_OK))
            {
                WiperCtrl_Sm_SelectFromStalk();
            }
            else
            {
                rainDemand = WiperCtrl_Rain_GetDemand(WiperCtrl_In.RainIntensity, WiperCtrl_In.IntLevel);
                switch (rainDemand)
                {
                    case WIPERCTRL_RAIN_HIGH:
                        WiperCtrl_MotorReq = MOTORSPEED_HIGH;
                        WiperCtrl_SwPhase  = SW_WAIT;
                        WiperCtrl_IntTimer = 0u;
                        break;
                    case WIPERCTRL_RAIN_LOW:
                        WiperCtrl_MotorReq = MOTORSPEED_LOW;
                        WiperCtrl_SwPhase  = SW_WAIT;
                        WiperCtrl_IntTimer = 0u;
                        break;
                    case WIPERCTRL_RAIN_INTERVAL:
                        WiperCtrl_MotorReq = WiperCtrl_Sm_SingleWipe(WiperCtrl_RainDelay);
                        break;
                    default:
                        /* no rain: finish running wipe, next interval demand wipes at once */
                        WiperCtrl_MotorReq = (WiperCtrl_In.ParkPos == TRUE) ? MOTORSPEED_STOP : MOTORSPEED_LOW;
                        WiperCtrl_SwPhase  = SW_WAIT;
                        WiperCtrl_IntTimer = 0xFFFFu;
                        break;
                }
            }
            break;

        case WIPER_ST_LOW:
            WiperCtrl_MotorReq = MOTORSPEED_LOW;
            if (enable == FALSE)
            {
                WiperCtrl_State = 6u;   /* parking */
            }
            else if (washActive == TRUE)
            {
                WiperCtrl_State = WIPER_ST_WASH;
            }
            else if (WiperCtrl_In.WiperPos != WIPERPOS_LOW)
            {
                WiperCtrl_Sm_SelectFromStalk();
            }
            else
            {
                /* stay */
            }
            break;

        case WIPER_ST_HIGH:
            WiperCtrl_MotorReq = MOTORSPEED_HIGH;
#if (WIPERCTRL_SPEED_DEPENDENT == STD_ON)
            /* reduce to low speed at standstill (< 3 km/h) */
            if (Rte_IRead_RE_WiperCtrl_MainFunction_VehicleSpeed_Speed() < 30u)
            {
                WiperCtrl_MotorReq = MOTORSPEED_LOW;
            }
#endif
            if (enable == FALSE)
            {
                WiperCtrl_State = 6u;   /* parking */
            }
            else if (washActive == TRUE)
            {
                WiperCtrl_State = WIPER_ST_WASH;
            }
            else if (WiperCtrl_In.WiperPos != WIPERPOS_HIGH)
            {
                WiperCtrl_Sm_SelectFromStalk();
            }
            else
            {
                /* stay */
            }
            break;

        case WIPER_ST_WASH:
            WiperCtrl_MotorReq = (WiperCtrl_In.WiperPos == WIPERPOS_HIGH) ? MOTORSPEED_HIGH : MOTORSPEED_LOW;
            if (enable == FALSE)
            {
                WiperCtrl_State = 6u;   /* parking */
            }
            else if (washActive == FALSE)
            {
                WiperCtrl_Sm_SelectFromStalk();
            }
            else
            {
                /* washing */
            }
            break;

        case WIPER_ST_PARKING:
            if (WiperCtrl_In.ParkPos == TRUE)
            {
                WiperCtrl_MotorReq = MOTORSPEED_STOP;
                WiperCtrl_State    = WIPER_ST_OFF;
            }
            else
            {
                WiperCtrl_MotorReq = MOTORSPEED_LOW;
                if ((enable == TRUE) && (washActive == TRUE))
                {
                    WiperCtrl_State = WIPER_ST_WASH;
                }
                else if ((enable == TRUE) && (WiperCtrl_In.WiperPos != WIPERPOS_OFF))
                {
                    WiperCtrl_Sm_SelectFromStalk();
                }
                else
                {
                    /* keep parking */
                }
            }
            break;

        case WIPER_ST_BLOCKED:
            /* motor protection owns the motor, see WiperCtrl_Motor.c */
            WiperCtrl_MotorReq = MOTORSPEED_STOP;
            if (WiperCtrl_MotorBlocked == FALSE)
            {
                if (enable == TRUE)
                {
                    WiperCtrl_Sm_SelectFromStalk();
                }
                else
                {
                    WiperCtrl_State = WIPER_ST_PARKING;
                }
            }
            break;

        case WIPER_ST_SERVICE:
            /* service position for blade change, requested by diagnostic routine */
            WiperCtrl_MotorReq = MOTORSPEED_STOP;
            break;

        default:
            WiperCtrl_State    = WIPER_ST_OFF;
            WiperCtrl_MotorReq = MOTORSPEED_STOP;
            break;
    }
}

/*------------------------------------------------------------------------------
 * select new state from stalk position (used by several states)
 *----------------------------------------------------------------------------*/
STATIC FUNC(void, WiperCtrl_CODE) WiperCtrl_Sm_SelectFromStalk(void)
{
    WiperCtrl_StateType prev = WiperCtrl_State;

    switch (WiperCtrl_In.WiperPos)
    {
        case WIPERPOS_INT:
            if (WiperCtrl_In.RainStatus == RAINSENSOR_OK)
            {
                WiperCtrl_State = WIPER_ST_AUTO;
            }
            else
            {
                WiperCtrl_State = WIPER_ST_INTERVAL;
            }
            break;

        case WIPERPOS_LOW:
            WiperCtrl_State = WIPER_ST_LOW;
            break;

        case WIPERPOS_HIGH:
            WiperCtrl_State = WIPER_ST_HIGH;
            break;

        default:    /* OFF or invalid */
            WiperCtrl_State = (WiperCtrl_In.ParkPos == TRUE) ? WIPER_ST_OFF : WIPER_ST_PARKING;
            break;
    }

    if (WiperCtrl_State != prev)
    {
        if (WiperCtrl_State == WIPER_ST_INTERVAL)
        {
            WiperCtrl_IntTimer = 0xFFFFu;   /* first wipe immediately */
            WiperCtrl_SwPhase  = SW_WAIT;
        }
        else if (WiperCtrl_State == WIPER_ST_AUTO)
        {
            WiperCtrl_IntTimer = 0u;
            WiperCtrl_SwPhase  = SW_WAIT;
        }
        else
        {
            /* nothing */
        }
    }
}

/*------------------------------------------------------------------------------
 * one wipe every 'period' [10 ms] measured from park position
 *----------------------------------------------------------------------------*/
STATIC FUNC(MotorSpeedCmd_T, WiperCtrl_CODE) WiperCtrl_Sm_SingleWipe(uint16 period)
{
    MotorSpeedCmd_T cmd = MOTORSPEED_STOP;

    switch (WiperCtrl_SwPhase)
    {
        case SW_WAIT:
            if (WiperCtrl_In.ParkPos == FALSE)
            {
                cmd = MOTORSPEED_LOW;       /* finish running wipe first */
                WiperCtrl_IntTimer = 0u;
            }
            else
            {
                if (WiperCtrl_IntTimer < 0xFFFFu)
                {
                    WiperCtrl_IntTimer++;
                }
                if (WiperCtrl_IntTimer >= period)
                {
                    WiperCtrl_SwPhase = SW_START;
                    cmd = MOTORSPEED_LOW;
                }
            }
            break;

        case SW_START:
            cmd = MOTORSPEED_LOW;
            if (WiperCtrl_In.ParkPos == FALSE)
            {
                WiperCtrl_SwPhase = SW_RUN;
            }
            break;

        case SW_RUN:
            cmd = MOTORSPEED_LOW;
            if (WiperCtrl_In.ParkPos == TRUE)
            {
                WiperCtrl_SwPhase  = SW_WAIT;
                WiperCtrl_IntTimer = 0u;
                cmd = MOTORSPEED_STOP;
            }
            break;

        default:
            WiperCtrl_SwPhase = SW_WAIT;
            break;
    }
    return cmd;
}

#define WiperCtrl_STOP_SEC_CODE
#include "WiperCtrl_MemMap.h"
