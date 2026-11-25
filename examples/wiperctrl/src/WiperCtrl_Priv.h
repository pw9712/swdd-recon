/*******************************************************************************
 *  Project     : BCM Front Wiper
 *  Component   : WiperCtrl (Application SWC)
 *  File        : WiperCtrl_Priv.h
 *  Description : Internal declarations shared between the WiperCtrl files.
 ******************************************************************************/
#ifndef WIPERCTRL_PRIV_H
#define WIPERCTRL_PRIV_H

#include "Rte_WiperCtrl.h"
#include "WiperCtrl_Cfg.h"

/* main state machine states */
typedef enum
{
    WIPER_ST_OFF = 0,
    WIPER_ST_INTERVAL,      /* 1 */
    WIPER_ST_LOW,           /* 2 */
    WIPER_ST_HIGH,          /* 3 */
    WIPER_ST_AUTO,          /* 4 */
    WIPER_ST_WASH,          /* 5 */
    WIPER_ST_PARKING,       /* 6 */
    WIPER_ST_BLOCKED,       /* 7 */
    WIPER_ST_SERVICE        /* 8 */
} WiperCtrl_StateType;

/* rain module demand */
#define WIPERCTRL_RAIN_NONE         0u
#define WIPERCTRL_RAIN_INTERVAL     1u
#define WIPERCTRL_RAIN_LOW          2u
#define WIPERCTRL_RAIN_HIGH         3u

/* snapshot of all RTE inputs, taken once per 10 ms cycle */
typedef struct
{
    WiperPos_T          WiperPos;
    IntLevel_T          IntLevel;
    boolean             WashReq;
    RainIntensity_T     RainIntensity;
    RainSensorStatus_T  RainStatus;
    PowerMode_T         PowerMode;
    boolean             ParkPos;
    MotorCurrent_T      MotorCurrent;
    boolean             HoodOpen;
} WiperCtrl_InputType;

/* ---- shared data (defined in WiperCtrl.c / WiperCtrl_Rain.c) ---------------- */
extern VAR(WiperCtrl_InputType, WiperCtrl_VAR) WiperCtrl_In;
extern VAR(WiperCtrl_StateType, WiperCtrl_VAR) WiperCtrl_State;
extern VAR(MotorSpeedCmd_T, WiperCtrl_VAR)     WiperCtrl_MotorReq;
extern VAR(boolean, WiperCtrl_VAR)             WiperCtrl_MotorBlocked;
extern VAR(boolean, WiperCtrl_VAR)             WiperCtrl_InitDone;
extern VAR(uint16, WiperCtrl_VAR)              WiperCtrl_RainDelay;

/* ---- WiperCtrl_Sm.c ---------------------------------------------------------- */
extern FUNC(void, WiperCtrl_CODE) WiperCtrl_Sm_Init(void);
extern FUNC(void, WiperCtrl_CODE) WiperCtrl_Sm_Cyclic(void);

/* ---- WiperCtrl_Motor.c ------------------------------------------------------- */
extern FUNC(void, WiperCtrl_CODE) WiperCtrl_Motor_Init(void);
extern FUNC(void, WiperCtrl_CODE) WiperCtrl_Motor_Cyclic(MotorSpeedCmd_T req);

/* ---- WiperCtrl_Wash.c -------------------------------------------------------- */
extern FUNC(void, WiperCtrl_CODE)    WiperCtrl_Wash_Init(void);
extern FUNC(boolean, WiperCtrl_CODE) WiperCtrl_Wash_Cyclic(boolean washReq, boolean enable);

/* ---- WiperCtrl_Rain.c -------------------------------------------------------- */
extern FUNC(void, WiperCtrl_CODE)  WiperCtrl_Rain_Init(void);
extern FUNC(uint8, WiperCtrl_CODE) WiperCtrl_Rain_GetDemand(RainIntensity_T intensity, IntLevel_T level);

#endif /* WIPERCTRL_PRIV_H */
