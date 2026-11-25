/*******************************************************************************
 *  Project     : BCM Front Wiper
 *  Component   : WiperCtrl (Application SWC)
 *  File        : WiperCtrl_Cfg.h
 *  Description : Compile time configuration and tuning values.
 ******************************************************************************/
#ifndef WIPERCTRL_CFG_H
#define WIPERCTRL_CFG_H

#include "Std_Types.h"

/* ---- variants -------------------------------------------------------------- */
#define WIPERCTRL_DEV_ERROR_DETECT      STD_ON
#define WIPERCTRL_SPEED_DEPENDENT       STD_OFF     /* HIGH->LOW at standstill, disabled since SOP2 */

/* ---- motor protection ------------------------------------------------------ */
#define WIPERCTRL_BLOCK_CURRENT         120u        /* 12.0 A (0.1 A/bit) */
#define WIPERCTRL_BLOCK_DEBOUNCE        30u         /* 500 ms */
#define WIPERCTRL_BLOCK_COOLDOWN        500u        /* 5 s */
#define WIPERCTRL_BLOCK_MAX_RETRY       3u

/* ---- washer ---------------------------------------------------------------- */
#define WIPERCTRL_WASH_PUMP_MAX         1000u       /* 10 s */
#define WIPERCTRL_WASH_AFTERWIPE_TICKS  150u        /* 3 s @ 20 ms task */

/* ---- rain sensor ----------------------------------------------------------- */
#define WIPERCTRL_RAIN_FAULT_DEBOUNCE   20u         /* 2 s @ 100 ms */
#define WIPERCTRL_AUTO_HYST             5u          /* % */

#endif /* WIPERCTRL_CFG_H */
