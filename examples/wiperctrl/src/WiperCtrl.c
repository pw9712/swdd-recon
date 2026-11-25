/*******************************************************************************
 *  Project     : BCM Front Wiper
 *  Component   : WiperCtrl (Application SWC)
 *  File        : WiperCtrl.c
 *  Description : Runnable entities, RTE input/output handling, diagnostics.
 *
 *  History
 *  Date        Author  Description
 *  ----------  ------  --------------------------------------------------------
 *  2014-03-12  JKO     Initial version (AUTOSAR 3.2 RTE)
 *  2016-11-02  JKO     Migration to AUTOSAR 4.2 RTE API
 *  2017-08-21  MZI     Rain sensor / AUTO mode integrated
 *  2019-05-14  PWA     Hood switch handling (pinch protection)
 *  2021-02-03  ASZ     NvM service counters
 ******************************************************************************/

#include "Rte_WiperCtrl.h"
#include "WiperCtrl.h"
#include "WiperCtrl_Priv.h"
#if (WIPERCTRL_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

/* =============================================================================
 * Data
 * ========================================================================== */
#define WiperCtrl_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "WiperCtrl_MemMap.h"

VAR(WiperCtrl_InputType, WiperCtrl_VAR) WiperCtrl_In;
VAR(WiperCtrl_StateType, WiperCtrl_VAR) WiperCtrl_State;

#define WiperCtrl_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "WiperCtrl_MemMap.h"

#define WiperCtrl_START_SEC_VAR_CLEARED_8
#include "WiperCtrl_MemMap.h"

VAR(MotorSpeedCmd_T, WiperCtrl_VAR) WiperCtrl_MotorReq;
VAR(boolean, WiperCtrl_VAR)         WiperCtrl_MotorBlocked;    /* set by motor protection */
VAR(boolean, WiperCtrl_VAR)         WiperCtrl_InitDone;

STATIC VAR(PowerMode_T, WiperCtrl_VAR) WiperCtrl_LastPowerMode;
STATIC VAR(uint8, WiperCtrl_VAR)       WiperCtrl_RainFaultCnt;
STATIC VAR(boolean, WiperCtrl_VAR)     WiperCtrl_BlockReported;

#define WiperCtrl_STOP_SEC_VAR_CLEARED_8
#include "WiperCtrl_MemMap.h"

/* =============================================================================
 * Code
 * ========================================================================== */
#define WiperCtrl_START_SEC_CODE
#include "WiperCtrl_MemMap.h"

STATIC FUNC(void, WiperCtrl_CODE) WiperCtrl_ReadInputs(void);
STATIC FUNC(void, WiperCtrl_CODE) WiperCtrl_WriteOutputs(void);
STATIC FUNC(WiperStatus_T, WiperCtrl_CODE) WiperCtrl_MapStatus(WiperCtrl_StateType st);

/*------------------------------------------------------------------------------
 * RE_WiperCtrl_Init
 *----------------------------------------------------------------------------*/
FUNC(void, WiperCtrl_CODE) RE_WiperCtrl_Init(void)
{
    NvM_RequestResultType nvmRes = NVM_REQ_NOT_OK;
    P2VAR(WiperCtrl_NvBlockType, AUTOMATIC, WiperCtrl_VAR) nv = Rte_Pim_NvMirror();

    /* mirror was filled by NvM_ReadAll - check result */
    (void)Rte_Call_NvMService_WiperCfg_GetErrorStatus(&nvmRes);
    if (nvmRes != NVM_REQ_OK)
    {
        nv->WashCycleCnt  = 0u;
        nv->BlockEventCnt = 0u;
        nv->Reserved      = 0u;
    }

    WiperCtrl_In.WiperPos      = WIPERPOS_OFF;
    WiperCtrl_In.IntLevel      = 1u;
    WiperCtrl_In.WashReq       = FALSE;
    WiperCtrl_In.RainIntensity = 0u;
    WiperCtrl_In.RainStatus    = RAINSENSOR_INIT;
    WiperCtrl_In.PowerMode     = POWERMODE_OFF;
    WiperCtrl_In.ParkPos       = TRUE;
    WiperCtrl_In.MotorCurrent  = 0u;
    WiperCtrl_In.HoodOpen      = FALSE;

    WiperCtrl_MotorReq      = MOTORSPEED_STOP;
    WiperCtrl_MotorBlocked  = FALSE;
    WiperCtrl_LastPowerMode = POWERMODE_OFF;
    WiperCtrl_RainFaultCnt  = 0u;
    WiperCtrl_BlockReported = FALSE;

    WiperCtrl_Sm_Init();
    WiperCtrl_Motor_Init();
    WiperCtrl_Wash_Init();
    WiperCtrl_Rain_Init();

    WiperCtrl_InitDone = TRUE;
}

/*------------------------------------------------------------------------------
 * RE_WiperCtrl_MainFunction - 10 ms
 *----------------------------------------------------------------------------*/
FUNC(void, WiperCtrl_CODE) RE_WiperCtrl_MainFunction(void)
{
#if (WIPERCTRL_DEV_ERROR_DETECT == STD_ON)
    if (WiperCtrl_InitDone != TRUE)
    {
        (void)Det_ReportError(WIPERCTRL_MODULE_ID, 0u, WIPERCTRL_SID_MAINFUNCTION, WIPERCTRL_E_UNINIT);
        return;
    }
#endif

    WiperCtrl_ReadInputs();
    WiperCtrl_Sm_Cyclic();
    WiperCtrl_Motor_Cyclic(WiperCtrl_MotorReq);
    WiperCtrl_WriteOutputs();
}

/*------------------------------------------------------------------------------
 * RE_WiperCtrl_Background - 100 ms
 * diagnostics and NvM handling
 *----------------------------------------------------------------------------*/
FUNC(void, WiperCtrl_CODE) RE_WiperCtrl_Background(void)
{
    NvM_RequestResultType nvmRes = NVM_REQ_OK;
    P2VAR(WiperCtrl_NvBlockType, AUTOMATIC, WiperCtrl_VAR) nv = Rte_Pim_NvMirror();

    /* --- rain sensor --- */
    if (WiperCtrl_In.RainStatus == RAINSENSOR_FAULT)
    {
        if (WiperCtrl_RainFaultCnt < WIPERCTRL_RAIN_FAULT_DEBOUNCE)
        {
            WiperCtrl_RainFaultCnt++;
        }
        else
        {
            (void)Rte_Call_Event_RainSensorFault_SetEventStatus(DEM_EVENT_STATUS_FAILED);
        }
    }
    else if (WiperCtrl_In.RainStatus == RAINSENSOR_OK)
    {
        WiperCtrl_RainFaultCnt = 0u;
        (void)Rte_Call_Event_RainSensorFault_SetEventStatus(DEM_EVENT_STATUS_PASSED);
    }
    else
    {
        /* sensor still initialising - no qualification */
    }

    /* --- motor blocking --- */
    if ((WiperCtrl_MotorBlocked == TRUE) && (WiperCtrl_BlockReported == FALSE))
    {
        (void)Rte_Call_Event_MotorBlocked_SetEventStatus(DEM_EVENT_STATUS_FAILED);
        WiperCtrl_BlockReported = TRUE;
        if (nv->BlockEventCnt < 0xFFu)
        {
            nv->BlockEventCnt++;
        }
    }
    else if ((WiperCtrl_MotorBlocked == FALSE) && (WiperCtrl_BlockReported == TRUE))
    {
        (void)Rte_Call_Event_MotorBlocked_SetEventStatus(DEM_EVENT_STATUS_PASSED);
        WiperCtrl_BlockReported = FALSE;
    }
    else
    {
        /* no change */
    }

    /* --- store counters when leaving RUN --- */
    if ((WiperCtrl_LastPowerMode == POWERMODE_RUN) && (WiperCtrl_In.PowerMode != POWERMODE_RUN))
    {
        (void)Rte_Call_NvMService_WiperCfg_GetErrorStatus(&nvmRes);
        if (nvmRes != NVM_REQ_PENDING)
        {
            (void)Rte_Call_NvMService_WiperCfg_WriteBlock(nv);
        }
    }
    WiperCtrl_LastPowerMode = WiperCtrl_In.PowerMode;
}

/*------------------------------------------------------------------------------
 * local functions
 *----------------------------------------------------------------------------*/
STATIC FUNC(void, WiperCtrl_CODE) WiperCtrl_ReadInputs(void)
{
    Std_ReturnType ret;

    (void)Rte_Read_StalkSwitch_WiperPos(&WiperCtrl_In.WiperPos);
    (void)Rte_Read_StalkSwitch_IntLevel(&WiperCtrl_In.IntLevel);
    (void)Rte_Read_StalkSwitch_WashReq(&WiperCtrl_In.WashReq);
    (void)Rte_Read_PowerMode_Mode(&WiperCtrl_In.PowerMode);
    (void)Rte_Read_WiperMotorFb_ParkPos(&WiperCtrl_In.ParkPos);
    (void)Rte_Read_WiperMotorFb_Current(&WiperCtrl_In.MotorCurrent);
    (void)Rte_Read_HoodStatus_Open(&WiperCtrl_In.HoodOpen);

    ret = Rte_Read_RainSensor_Intensity(&WiperCtrl_In.RainIntensity);
    if (ret != RTE_E_OK)
    {
        /* LIN timeout / never received */
        WiperCtrl_In.RainStatus = RAINSENSOR_FAULT;
    }
    else
    {
        (void)Rte_Read_RainSensor_Status(&WiperCtrl_In.RainStatus);
    }
}

STATIC FUNC(void, WiperCtrl_CODE) WiperCtrl_WriteOutputs(void)
{
    /* motor command is written by WiperCtrl_Motor_Cyclic() */
    (void)Rte_Write_WiperStatus_Mode(WiperCtrl_MapStatus(WiperCtrl_State));
}

STATIC FUNC(WiperStatus_T, WiperCtrl_CODE) WiperCtrl_MapStatus(WiperCtrl_StateType st)
{
    WiperStatus_T ret;

    switch (st)
    {
        case WIPER_ST_INTERVAL: ret = WIPERSTATUS_INTERVAL; break;
        case WIPER_ST_AUTO:     ret = WIPERSTATUS_AUTO;     break;
        case WIPER_ST_LOW:      ret = WIPERSTATUS_LOW;      break;
        case WIPER_ST_HIGH:     ret = WIPERSTATUS_HIGH;     break;
        case WIPER_ST_WASH:     ret = WIPERSTATUS_WASH;     break;
        case WIPER_ST_BLOCKED:  ret = WIPERSTATUS_FAULT;    break;
        default:                ret = WIPERSTATUS_OFF;      break;  /* OFF, PARKING, SERVICE */
    }
    return ret;
}

#define WiperCtrl_STOP_SEC_CODE
#include "WiperCtrl_MemMap.h"
