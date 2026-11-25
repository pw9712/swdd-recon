/*******************************************************************************
 *  Project     : BCM Front Wiper
 *  Component   : WiperCtrl (Application SWC)
 *  File        : WiperCtrl_Rain.c
 *  Description : Rain sensor evaluation for AUTO mode.
 *
 *  History
 *  Date        Author  Description
 *  ----------  ------  --------------------------------------------------------
 *  2017-08-21  MZI     Initial version (SOP1: fixed thresholds)
 *  2018-10-11  MZI     Sensitivity via stalk interval ring, calibratable threshold
 ******************************************************************************/

#include "WiperCtrl_Priv.h"

/* =============================================================================
 * Data
 * ========================================================================== */
#define WiperCtrl_START_SEC_VAR_CLEARED_16
#include "WiperCtrl_MemMap.h"

VAR(uint16, WiperCtrl_VAR) WiperCtrl_RainDelay;     /* interval demand period [10 ms] */

#define WiperCtrl_STOP_SEC_VAR_CLEARED_16
#include "WiperCtrl_MemMap.h"

#define WiperCtrl_START_SEC_VAR_CLEARED_8
#include "WiperCtrl_MemMap.h"

STATIC VAR(uint8, WiperCtrl_VAR) WiperCtrl_RainLastDemand;

#define WiperCtrl_STOP_SEC_VAR_CLEARED_8
#include "WiperCtrl_MemMap.h"

/* =============================================================================
 * Code
 * ========================================================================== */
#define WiperCtrl_START_SEC_CODE
#include "WiperCtrl_MemMap.h"

FUNC(void, WiperCtrl_CODE) WiperCtrl_Rain_Init(void)
{
    WiperCtrl_RainLastDemand = WIPERCTRL_RAIN_NONE;
    WiperCtrl_RainDelay      = 0u;
}

/*------------------------------------------------------------------------------
 * WiperCtrl_Rain_GetDemand
 * intensity : rain intensity from sensor [%]
 * level     : stalk interval ring, used as sensitivity in AUTO
 * return    : WIPERCTRL_RAIN_xxx, for INTERVAL the period is in WiperCtrl_RainDelay
 *----------------------------------------------------------------------------*/
FUNC(uint8, WiperCtrl_CODE) WiperCtrl_Rain_GetDemand(RainIntensity_T intensity, IntLevel_T level)
{
    uint16 eff;
    uint16 rel;
    uint8  thr;
    uint8  lowThr;
    uint8  demand;

    if (level < 1u) { level = 1u; }
    if (level > 4u) { level = 4u; }

    /* sensitivity: +5 % per ring position */
    eff = (uint16)intensity + ((uint16)(level - 1u) * 5u);
    if (eff > 100u)
    {
        eff = 100u;
    }

    thr = Rte_CData_RainThreshold();

#if 0
    /* SOP1 algorithm - fixed thresholds, no sensitivity */
    if (intensity > 60u)
    {
        demand = WIPERCTRL_RAIN_HIGH;
    }
    else if (intensity > 30u)
    {
        demand = WIPERCTRL_RAIN_LOW;
    }
    else if (intensity > 5u)
    {
        demand = WIPERCTRL_RAIN_INTERVAL;
        WiperCtrl_RainDelay = 500u;
    }
    else
    {
        demand = WIPERCTRL_RAIN_NONE;
    }
#endif

    /* hysteresis when stepping down */
    lowThr = thr;
    if (WiperCtrl_RainLastDemand != WIPERCTRL_RAIN_NONE)
    {
        lowThr = (thr > WIPERCTRL_AUTO_HYST) ? (uint8)(thr - WIPERCTRL_AUTO_HYST) : 0u;
    }

    if (eff < lowThr)
    {
        demand = WIPERCTRL_RAIN_NONE;
    }
    else if (eff < ((WiperCtrl_RainLastDemand >= WIPERCTRL_RAIN_LOW) ? (50u - WIPERCTRL_AUTO_HYST) : 50u))
    {
        demand = WIPERCTRL_RAIN_INTERVAL;
        /* 8 s at threshold ... 1 s at 50 % */
        rel = (eff > thr) ? (uint16)(eff - thr) : 0u;
        WiperCtrl_RainDelay = (uint16)(800u - ((rel * 700u) / (50u - thr)));
    }
    else if (eff < ((WiperCtrl_RainLastDemand == WIPERCTRL_RAIN_HIGH) ? (80u - WIPERCTRL_AUTO_HYST) : 80u))
    {
        demand = WIPERCTRL_RAIN_LOW;
    }
    else
    {
        demand = WIPERCTRL_RAIN_HIGH;
    }

    WiperCtrl_RainLastDemand = demand;
    return demand;
}

#define WiperCtrl_STOP_SEC_CODE
#include "WiperCtrl_MemMap.h"
