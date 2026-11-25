/**
 * \file   Rte_Stub.h
 * \brief  Host RTE stub - test access to the simulated port values.
 */
#ifndef RTE_STUB_H
#define RTE_STUB_H

#include "Rte_WiperCtrl_Type.h"

#define RTESTUB_DEM_NOT_REPORTED  ((Dem_EventStatusType)0xFFu)

/* inputs (set by the test) */
extern WiperPos_T          RteStub_StalkSwitch_WiperPos;
extern IntLevel_T          RteStub_StalkSwitch_IntLevel;
extern boolean             RteStub_StalkSwitch_WashReq;
extern RainIntensity_T     RteStub_RainSensor_Intensity;
extern Std_ReturnType      RteStub_RainSensor_IntensityRet;
extern RainSensorStatus_T  RteStub_RainSensor_Status;
extern PowerMode_T         RteStub_PowerMode_Mode;
extern boolean             RteStub_WiperMotorFb_ParkPos;
extern MotorCurrent_T      RteStub_WiperMotorFb_Current;
extern boolean             RteStub_HoodStatus_Open;
extern VehSpeed_T          RteStub_VehicleSpeed_Speed;
extern uint8               RteStub_RainThreshold;
extern NvM_RequestResultType RteStub_NvM_ErrorStatus;

/* outputs (observed by the test) */
extern MotorSpeedCmd_T     RteStub_WiperMotorCmd_Speed;
extern WiperStatus_T       RteStub_WiperStatus_Mode;
extern boolean             RteStub_WasherPump_On;
extern unsigned int        RteStub_WasherPump_CallCnt;
extern Dem_EventStatusType RteStub_Dem_MotorBlocked;
extern Dem_EventStatusType RteStub_Dem_RainSensorFault;
extern unsigned int        RteStub_NvM_WriteCnt;
extern WiperCtrl_NvBlockType RteStub_NvMirror;
extern unsigned int        RteStub_DetErrorCnt;

void RteStub_Reset(void);

#endif /* RTE_STUB_H */
