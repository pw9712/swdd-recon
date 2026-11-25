/**
 * \file   Rte_Stub.c
 * \brief  Host implementation of the WiperCtrl RTE API and DET stub.
 */
#include "Rte_WiperCtrl.h"
#include "Det.h"
#include "Rte_Stub.h"

WiperPos_T            RteStub_StalkSwitch_WiperPos;
IntLevel_T            RteStub_StalkSwitch_IntLevel;
boolean               RteStub_StalkSwitch_WashReq;
RainIntensity_T       RteStub_RainSensor_Intensity;
Std_ReturnType        RteStub_RainSensor_IntensityRet;
RainSensorStatus_T    RteStub_RainSensor_Status;
PowerMode_T           RteStub_PowerMode_Mode;
boolean               RteStub_WiperMotorFb_ParkPos;
MotorCurrent_T        RteStub_WiperMotorFb_Current;
boolean               RteStub_HoodStatus_Open;
VehSpeed_T            RteStub_VehicleSpeed_Speed;
uint8                 RteStub_RainThreshold;
NvM_RequestResultType RteStub_NvM_ErrorStatus;

MotorSpeedCmd_T       RteStub_WiperMotorCmd_Speed;
WiperStatus_T         RteStub_WiperStatus_Mode;
boolean               RteStub_WasherPump_On;
unsigned int          RteStub_WasherPump_CallCnt;
Dem_EventStatusType   RteStub_Dem_MotorBlocked;
Dem_EventStatusType   RteStub_Dem_RainSensorFault;
unsigned int          RteStub_NvM_WriteCnt;
WiperCtrl_NvBlockType RteStub_NvMirror;
unsigned int          RteStub_DetErrorCnt;

void RteStub_Reset(void)
{
    RteStub_StalkSwitch_WiperPos    = WIPERPOS_OFF;
    RteStub_StalkSwitch_IntLevel    = 1u;
    RteStub_StalkSwitch_WashReq     = FALSE;
    RteStub_RainSensor_Intensity    = 0u;
    RteStub_RainSensor_IntensityRet = RTE_E_OK;
    RteStub_RainSensor_Status       = RAINSENSOR_OK;
    RteStub_PowerMode_Mode          = POWERMODE_OFF;
    RteStub_WiperMotorFb_ParkPos    = TRUE;
    RteStub_WiperMotorFb_Current    = 0u;
    RteStub_HoodStatus_Open         = FALSE;
    RteStub_VehicleSpeed_Speed      = 0u;
    RteStub_RainThreshold           = 10u;
    RteStub_NvM_ErrorStatus         = NVM_REQ_OK;

    RteStub_WiperMotorCmd_Speed     = MOTORSPEED_STOP;
    RteStub_WiperStatus_Mode        = WIPERSTATUS_OFF;
    RteStub_WasherPump_On           = FALSE;
    RteStub_WasherPump_CallCnt      = 0u;
    RteStub_Dem_MotorBlocked        = RTESTUB_DEM_NOT_REPORTED;
    RteStub_Dem_RainSensorFault     = RTESTUB_DEM_NOT_REPORTED;
    RteStub_NvM_WriteCnt            = 0u;
    RteStub_NvMirror.WashCycleCnt   = 41u;   /* "restored" by NvM_ReadAll */
    RteStub_NvMirror.BlockEventCnt  = 2u;
    RteStub_NvMirror.Reserved       = 0u;
    RteStub_DetErrorCnt             = 0u;
}

/* ---- S/R read ---- */
Std_ReturnType Rte_Read_StalkSwitch_WiperPos(WiperPos_T *data)           { *data = RteStub_StalkSwitch_WiperPos; return RTE_E_OK; }
Std_ReturnType Rte_Read_StalkSwitch_IntLevel(IntLevel_T *data)           { *data = RteStub_StalkSwitch_IntLevel; return RTE_E_OK; }
Std_ReturnType Rte_Read_StalkSwitch_WashReq(boolean *data)               { *data = RteStub_StalkSwitch_WashReq; return RTE_E_OK; }
Std_ReturnType Rte_Read_RainSensor_Intensity(RainIntensity_T *data)      { *data = RteStub_RainSensor_Intensity; return RteStub_RainSensor_IntensityRet; }
Std_ReturnType Rte_Read_RainSensor_Status(RainSensorStatus_T *data)      { *data = RteStub_RainSensor_Status; return RTE_E_OK; }
Std_ReturnType Rte_Read_PowerMode_Mode(PowerMode_T *data)                { *data = RteStub_PowerMode_Mode; return RTE_E_OK; }
Std_ReturnType Rte_Read_WiperMotorFb_ParkPos(boolean *data)              { *data = RteStub_WiperMotorFb_ParkPos; return RTE_E_OK; }
Std_ReturnType Rte_Read_WiperMotorFb_Current(MotorCurrent_T *data)       { *data = RteStub_WiperMotorFb_Current; return RTE_E_OK; }
Std_ReturnType Rte_Read_HoodStatus_Open(boolean *data)                   { *data = RteStub_HoodStatus_Open; return RTE_E_OK; }
VehSpeed_T     Rte_IRead_RE_WiperCtrl_MainFunction_VehicleSpeed_Speed(void) { return RteStub_VehicleSpeed_Speed; }

/* ---- S/R write ---- */
Std_ReturnType Rte_Write_WiperMotorCmd_Speed(MotorSpeedCmd_T data)       { RteStub_WiperMotorCmd_Speed = data; return RTE_E_OK; }
Std_ReturnType Rte_Write_WiperStatus_Mode(WiperStatus_T data)            { RteStub_WiperStatus_Mode = data; return RTE_E_OK; }

/* ---- C/S ---- */
Std_ReturnType Rte_Call_WasherPump_SetState(boolean On)
{
    RteStub_WasherPump_On = On;
    RteStub_WasherPump_CallCnt++;
    return RTE_E_OK;
}
Std_ReturnType Rte_Call_Event_MotorBlocked_SetEventStatus(Dem_EventStatusType s)    { RteStub_Dem_MotorBlocked = s; return E_OK; }
Std_ReturnType Rte_Call_Event_RainSensorFault_SetEventStatus(Dem_EventStatusType s) { RteStub_Dem_RainSensorFault = s; return E_OK; }
Std_ReturnType Rte_Call_NvMService_WiperCfg_GetErrorStatus(NvM_RequestResultType *s) { *s = RteStub_NvM_ErrorStatus; return E_OK; }
Std_ReturnType Rte_Call_NvMService_WiperCfg_WriteBlock(const void *src)              { (void)src; RteStub_NvM_WriteCnt++; return E_OK; }

/* ---- PIM / calibration ---- */
WiperCtrl_NvBlockType *Rte_Pim_NvMirror(void) { return &RteStub_NvMirror; }
uint8 Rte_CData_RainThreshold(void)           { return RteStub_RainThreshold; }

/* ---- DET ---- */
Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId)
{
    (void)ModuleId; (void)InstanceId; (void)ApiId; (void)ErrorId;
    RteStub_DetErrorCnt++;
    return E_OK;
}
