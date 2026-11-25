/*******************************************************************************
 *  Project     : BCM Front Wiper
 *  Component   : WiperCtrl (Application SWC)
 *  File        : WiperCtrl.h
 *  Description : Public header - version information and DET ids.
 ******************************************************************************/
#ifndef WIPERCTRL_H
#define WIPERCTRL_H

#include "Std_Types.h"

#define WIPERCTRL_SW_MAJOR_VERSION      3u
#define WIPERCTRL_SW_MINOR_VERSION      4u
#define WIPERCTRL_SW_PATCH_VERSION      1u

/* project specific id used for DET reporting */
#define WIPERCTRL_MODULE_ID             ((uint16)500u)

/* service ids */
#define WIPERCTRL_SID_INIT              0x01u
#define WIPERCTRL_SID_MAINFUNCTION      0x02u
#define WIPERCTRL_SID_BACKGROUND        0x03u

/* DET error ids */
#define WIPERCTRL_E_UNINIT              0x01u

#endif /* WIPERCTRL_H */
