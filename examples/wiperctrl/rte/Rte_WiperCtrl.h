/**
 * \file   Rte_WiperCtrl.h
 * \brief  RTE application header for SWC WiperCtrl.
 *
 * Hand-written stand-in for the RTE generator output. On the host the API is
 * implemented by test/Rte_Stub.c.
 */
#ifndef RTE_WIPERCTRL_H
#define RTE_WIPERCTRL_H

#include "Rte_WiperCtrl_Type.h"

/* ---------------------------------------------------------------------------
 * Sender/Receiver - explicit read
 * ------------------------------------------------------------------------- */
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Read_StalkSwitch_WiperPos(P2VAR(WiperPos_T, AUTOMATIC, RTE_APPL_DATA) data);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Read_StalkSwitch_IntLevel(P2VAR(IntLevel_T, AUTOMATIC, RTE_APPL_DATA) data);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Read_StalkSwitch_WashReq(P2VAR(boolean, AUTOMATIC, RTE_APPL_DATA) data);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Read_RainSensor_Intensity(P2VAR(RainIntensity_T, AUTOMATIC, RTE_APPL_DATA) data);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Read_RainSensor_Status(P2VAR(RainSensorStatus_T, AUTOMATIC, RTE_APPL_DATA) data);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Read_PowerMode_Mode(P2VAR(PowerMode_T, AUTOMATIC, RTE_APPL_DATA) data);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Read_WiperMotorFb_ParkPos(P2VAR(boolean, AUTOMATIC, RTE_APPL_DATA) data);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Read_WiperMotorFb_Current(P2VAR(MotorCurrent_T, AUTOMATIC, RTE_APPL_DATA) data);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Read_HoodStatus_Open(P2VAR(boolean, AUTOMATIC, RTE_APPL_DATA) data);

/* ---------------------------------------------------------------------------
 * Sender/Receiver - implicit read
 * ------------------------------------------------------------------------- */
extern FUNC(VehSpeed_T, RTE_CODE) Rte_IRead_RE_WiperCtrl_MainFunction_VehicleSpeed_Speed(void);

/* ---------------------------------------------------------------------------
 * Sender/Receiver - explicit write
 * ------------------------------------------------------------------------- */
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Write_WiperMotorCmd_Speed(MotorSpeedCmd_T data);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Write_WiperStatus_Mode(WiperStatus_T data);

/* ---------------------------------------------------------------------------
 * Client/Server
 * ------------------------------------------------------------------------- */
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Call_WasherPump_SetState(boolean On);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Call_Event_MotorBlocked_SetEventStatus(Dem_EventStatusType EventStatus);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Call_Event_RainSensorFault_SetEventStatus(Dem_EventStatusType EventStatus);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Call_NvMService_WiperCfg_GetErrorStatus(P2VAR(NvM_RequestResultType, AUTOMATIC, RTE_APPL_DATA) ErrorStatus);
extern FUNC(Std_ReturnType, RTE_CODE) Rte_Call_NvMService_WiperCfg_WriteBlock(P2CONST(void, AUTOMATIC, RTE_APPL_DATA) SrcPtr);

/* ---------------------------------------------------------------------------
 * Per-instance memory / calibration
 * ------------------------------------------------------------------------- */
extern FUNC_P2VAR(WiperCtrl_NvBlockType, AUTOMATIC, RTE_CODE) Rte_Pim_NvMirror(void);
extern FUNC(uint8, RTE_CODE) Rte_CData_RainThreshold(void);

/* ---------------------------------------------------------------------------
 * Runnable entities
 * ------------------------------------------------------------------------- */
extern FUNC(void, WiperCtrl_CODE) RE_WiperCtrl_Init(void);
extern FUNC(void, WiperCtrl_CODE) RE_WiperCtrl_MainFunction(void);
extern FUNC(void, WiperCtrl_CODE) RE_WiperCtrl_Background(void);

#endif /* RTE_WIPERCTRL_H */
