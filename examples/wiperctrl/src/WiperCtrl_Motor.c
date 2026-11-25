/*******************************************************************************
 *  Project     : BCM Front Wiper
 *  Component   : WiperCtrl (Application SWC)
 *  File        : WiperCtrl_Motor.c
 *  Description : Motor command output and blocking protection.
 *
 *  History
 *  Date        Author  Description
 *  ----------  ------  --------------------------------------------------------
 *  2014-03-12  JKO     Initial version
 *  2016-04-18  JKO     Blocking detection with retry (CR-1093)
 *  2018-01-22  MZI     Lock until driver re-activates stalk
 ******************************************************************************/

#include "WiperCtrl_Priv.h"

/* protection states */
#define WIPERCTRL_MOT_NORMAL    0u
#define WIPERCTRL_MOT_COOLDOWN  1u
#define WIPERCTRL_MOT_RETRY     2u
#define WIPERCTRL_MOT_LOCKED    3u

/* =============================================================================
 * Data
 * ========================================================================== */
#define WiperCtrl_START_SEC_VAR_CLEARED_16
#include "WiperCtrl_MemMap.h"

STATIC VAR(uint16, WiperCtrl_VAR) WiperCtrl_MotTimer;

#define WiperCtrl_STOP_SEC_VAR_CLEARED_16
#include "WiperCtrl_MemMap.h"

#define WiperCtrl_START_SEC_VAR_CLEARED_8
#include "WiperCtrl_MemMap.h"

STATIC VAR(uint8, WiperCtrl_VAR)           WiperCtrl_MotState;
STATIC VAR(uint8, WiperCtrl_VAR)           WiperCtrl_OverCurCnt;
STATIC VAR(uint8, WiperCtrl_VAR)           WiperCtrl_BlockRetryCnt;
STATIC VAR(boolean, WiperCtrl_VAR)         WiperCtrl_LastParkPos;
STATIC VAR(WiperPos_T, WiperCtrl_VAR)      WiperCtrl_MotLastPos;
STATIC VAR(MotorSpeedCmd_T, WiperCtrl_VAR) WiperCtrl_MotorOut;

#define WiperCtrl_STOP_SEC_VAR_CLEARED_8
#include "WiperCtrl_MemMap.h"

/* =============================================================================
 * Code
 * ========================================================================== */
#define WiperCtrl_START_SEC_CODE
#include "WiperCtrl_MemMap.h"

STATIC FUNC(void, WiperCtrl_CODE) WiperCtrl_Motor_EnterCooldown(void);

FUNC(void, WiperCtrl_CODE) WiperCtrl_Motor_Init(void)
{
    WiperCtrl_MotState      = WIPERCTRL_MOT_NORMAL;
    WiperCtrl_MotTimer      = 0u;
    WiperCtrl_OverCurCnt    = 0u;
    WiperCtrl_BlockRetryCnt = 0u;
    WiperCtrl_LastParkPos   = TRUE;
    WiperCtrl_MotLastPos    = WIPERPOS_OFF;
    WiperCtrl_MotorOut      = MOTORSPEED_STOP;
}

/*------------------------------------------------------------------------------
 * WiperCtrl_Motor_Cyclic - 10 ms
 * req: speed requested by the state machine
 *----------------------------------------------------------------------------*/
FUNC(void, WiperCtrl_CODE) WiperCtrl_Motor_Cyclic(MotorSpeedCmd_T req)
{
    boolean overCur;
    boolean parkRise;
    boolean stalkOn;

    /* over current debounce, only while motor is driven */
    if ((WiperCtrl_MotorOut != MOTORSPEED_STOP) && (WiperCtrl_In.MotorCurrent > WIPERCTRL_BLOCK_CURRENT))
    {
        if (WiperCtrl_OverCurCnt < 0xFFu)
        {
            WiperCtrl_OverCurCnt++;
        }
    }
    else
    {
        WiperCtrl_OverCurCnt = 0u;
    }
    overCur = (WiperCtrl_OverCurCnt >= WIPERCTRL_BLOCK_DEBOUNCE) ? TRUE : FALSE;

    parkRise = ((WiperCtrl_In.ParkPos == TRUE) && (WiperCtrl_LastParkPos == FALSE)) ? TRUE : FALSE;
    WiperCtrl_LastParkPos = WiperCtrl_In.ParkPos;

    stalkOn = ((WiperCtrl_In.WiperPos != WIPERPOS_OFF) && (WiperCtrl_MotLastPos == WIPERPOS_OFF)) ? TRUE : FALSE;
    WiperCtrl_MotLastPos = WiperCtrl_In.WiperPos;

    switch (WiperCtrl_MotState)
    {
        case WIPERCTRL_MOT_NORMAL:
            WiperCtrl_MotorOut = req;
            if (overCur == TRUE)
            {
                WiperCtrl_BlockRetryCnt = 0u;
                WiperCtrl_MotorBlocked  = TRUE;
                WiperCtrl_Motor_EnterCooldown();
            }
            break;

        case WIPERCTRL_MOT_COOLDOWN:
            WiperCtrl_MotorOut = MOTORSPEED_STOP;
            WiperCtrl_MotTimer++;
            if (WiperCtrl_MotTimer >= WIPERCTRL_BLOCK_COOLDOWN)
            {
                if (WiperCtrl_BlockRetryCnt < WIPERCTRL_BLOCK_MAX_RETRY)
                {
                    WiperCtrl_BlockRetryCnt++;
                    WiperCtrl_MotState = WIPERCTRL_MOT_RETRY;
                }
                else
                {
                    WiperCtrl_MotState = WIPERCTRL_MOT_LOCKED;
                }
            }
            break;

        case WIPERCTRL_MOT_RETRY:
            /* retry always at low speed, independent of request */
            WiperCtrl_MotorOut = MOTORSPEED_LOW;
            if (overCur == TRUE)
            {
                WiperCtrl_Motor_EnterCooldown();
            }
            else if (parkRise == TRUE)
            {
                /* one full wipe done - motor is free again */
                WiperCtrl_MotState      = WIPERCTRL_MOT_NORMAL;
                WiperCtrl_BlockRetryCnt = 0u;
                WiperCtrl_MotorBlocked  = FALSE;
            }
            else
            {
                /* retry running */
            }
            break;

        case WIPERCTRL_MOT_LOCKED:
            WiperCtrl_MotorOut = MOTORSPEED_STOP;
            if (stalkOn == TRUE)
            {
                /* driver re-activated the wiper: new attempt */
                WiperCtrl_MotState      = WIPERCTRL_MOT_NORMAL;
                WiperCtrl_BlockRetryCnt = 0u;
                WiperCtrl_MotorBlocked  = FALSE;
            }
            break;

        default:
            WiperCtrl_MotState = WIPERCTRL_MOT_NORMAL;
            WiperCtrl_MotorOut = MOTORSPEED_STOP;
            break;
    }

    (void)Rte_Write_WiperMotorCmd_Speed(WiperCtrl_MotorOut);
}

STATIC FUNC(void, WiperCtrl_CODE) WiperCtrl_Motor_EnterCooldown(void)
{
    WiperCtrl_MotState   = WIPERCTRL_MOT_COOLDOWN;
    WiperCtrl_MotTimer   = 0u;
    WiperCtrl_OverCurCnt = 0u;
    WiperCtrl_MotorOut   = MOTORSPEED_STOP;
}

#define WiperCtrl_STOP_SEC_CODE
#include "WiperCtrl_MemMap.h"
