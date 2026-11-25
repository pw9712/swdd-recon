/*******************************************************************************
 *  Project     : BCM Front Wiper
 *  Component   : WiperCtrl (Application SWC)
 *  File        : WiperCtrl_Wash.c
 *  Description : Wash / wipe function (washer pump and after-wipe).
 *
 *  History
 *  Date        Author  Description
 *  ----------  ------  --------------------------------------------------------
 *  2014-05-07  JKO     Initial version (20 ms task)
 *  2016-11-02  JKO     Moved to 10 ms main runnable
 *  2021-02-03  ASZ     Wash cycle counter for NvM
 ******************************************************************************/

#include "WiperCtrl_Priv.h"

/* wash phases */
#define WIPERCTRL_WASH_IDLE         0u
#define WIPERCTRL_WASH_PUMP         1u
#define WIPERCTRL_WASH_AFTERWIPE    2u

/* =============================================================================
 * Data
 * ========================================================================== */
#define WiperCtrl_START_SEC_VAR_CLEARED_16
#include "WiperCtrl_MemMap.h"

STATIC VAR(uint16, WiperCtrl_VAR) WiperCtrl_WashTimer;

#define WiperCtrl_STOP_SEC_VAR_CLEARED_16
#include "WiperCtrl_MemMap.h"

#define WiperCtrl_START_SEC_VAR_CLEARED_8
#include "WiperCtrl_MemMap.h"

STATIC VAR(uint8, WiperCtrl_VAR)   WiperCtrl_WashPhase;
STATIC VAR(boolean, WiperCtrl_VAR) WiperCtrl_PumpOn;

#define WiperCtrl_STOP_SEC_VAR_CLEARED_8
#include "WiperCtrl_MemMap.h"

/* =============================================================================
 * Code
 * ========================================================================== */
#define WiperCtrl_START_SEC_CODE
#include "WiperCtrl_MemMap.h"

STATIC FUNC(void, WiperCtrl_CODE) WiperCtrl_Wash_SetPump(boolean on);

FUNC(void, WiperCtrl_CODE) WiperCtrl_Wash_Init(void)
{
    WiperCtrl_WashPhase = WIPERCTRL_WASH_IDLE;
    WiperCtrl_WashTimer = 0u;
    WiperCtrl_PumpOn    = FALSE;
}

/*------------------------------------------------------------------------------
 * WiperCtrl_Wash_Cyclic
 * washReq : wash button pressed
 * enable  : wash allowed (power mode RUN, not blocked)
 * return  : TRUE while the wash sequence needs the wipers
 *----------------------------------------------------------------------------*/
FUNC(boolean, WiperCtrl_CODE) WiperCtrl_Wash_Cyclic(boolean washReq, boolean enable)
{
    boolean active = FALSE;
    P2VAR(WiperCtrl_NvBlockType, AUTOMATIC, WiperCtrl_VAR) nv;

    if (enable == FALSE)
    {
        WiperCtrl_Wash_SetPump(FALSE);
        WiperCtrl_WashPhase = WIPERCTRL_WASH_IDLE;
        return FALSE;
    }

    switch (WiperCtrl_WashPhase)
    {
        case WIPERCTRL_WASH_IDLE:
            if (washReq == TRUE)
            {
                WiperCtrl_WashPhase = WIPERCTRL_WASH_PUMP;
                WiperCtrl_WashTimer = 0u;
                WiperCtrl_Wash_SetPump(TRUE);
                active = TRUE;
            }
            break;

        case WIPERCTRL_WASH_PUMP:
            active = TRUE;
            WiperCtrl_WashTimer++;
            if ((washReq == FALSE) || (WiperCtrl_WashTimer >= WIPERCTRL_WASH_PUMP_MAX))
            {
                WiperCtrl_Wash_SetPump(FALSE);
                WiperCtrl_WashPhase = WIPERCTRL_WASH_AFTERWIPE;
                WiperCtrl_WashTimer = 0u;
            }
            break;

        case WIPERCTRL_WASH_AFTERWIPE:
            active = TRUE;
            WiperCtrl_WashTimer++;
            if (WiperCtrl_WashTimer >= WIPERCTRL_WASH_AFTERWIPE_TICKS)
            {
                WiperCtrl_WashPhase = WIPERCTRL_WASH_IDLE;
                nv = Rte_Pim_NvMirror();
                if (nv->WashCycleCnt < 0xFFFFu)
                {
                    nv->WashCycleCnt++;
                }
                active = FALSE;
            }
            break;
    }

    return active;
}

STATIC FUNC(void, WiperCtrl_CODE) WiperCtrl_Wash_SetPump(boolean on)
{
    if (on != WiperCtrl_PumpOn)
    {
        (void)Rte_Call_WasherPump_SetState(on);
        WiperCtrl_PumpOn = on;
    }
}

#define WiperCtrl_STOP_SEC_CODE
#include "WiperCtrl_MemMap.h"
