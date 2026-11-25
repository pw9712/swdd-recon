/**
 * \file   Rte_WiperCtrl_Type.h
 * \brief  RTE type definitions for SWC WiperCtrl.
 *
 * Hand-written stand-in for the RTE generator output (contract between the
 * SWC and the RTE). Do not edit in a real project - regenerate from ARXML.
 */
#ifndef RTE_WIPERCTRL_TYPE_H
#define RTE_WIPERCTRL_TYPE_H

#include "Std_Types.h"

/* ---------------------------------------------------------------------------
 * RTE status codes
 * ------------------------------------------------------------------------- */
#define RTE_E_OK                ((Std_ReturnType)0u)
#define RTE_E_INVALID           ((Std_ReturnType)1u)
#define RTE_E_COM_STOPPED       ((Std_ReturnType)128u)
#define RTE_E_TIMEOUT           ((Std_ReturnType)129u)
#define RTE_E_LIMIT             ((Std_ReturnType)130u)
#define RTE_E_NO_DATA           ((Std_ReturnType)131u)
#define RTE_E_TRANSMIT_ACK      ((Std_ReturnType)132u)
#define RTE_E_NEVER_RECEIVED    ((Std_ReturnType)133u)

/* ---------------------------------------------------------------------------
 * Application data types
 * ------------------------------------------------------------------------- */
typedef uint8 WiperPos_T;               /* CM_WiperPos (TEXTTABLE) */
#define WIPERPOS_OFF            0u
#define WIPERPOS_INT            1u
#define WIPERPOS_LOW            2u
#define WIPERPOS_HIGH           3u

typedef uint8 IntLevel_T;               /* 1..4 */

typedef uint8 RainIntensity_T;          /* 0..100 % */

typedef uint8 RainSensorStatus_T;       /* CM_RainSensorStatus (TEXTTABLE) */
#define RAINSENSOR_OK           0u
#define RAINSENSOR_INIT         1u
#define RAINSENSOR_FAULT        2u

typedef uint8 PowerMode_T;              /* CM_PowerMode (TEXTTABLE) */
#define POWERMODE_OFF           0u
#define POWERMODE_ACC           1u
#define POWERMODE_RUN           2u
#define POWERMODE_CRANK         3u

typedef uint16 MotorCurrent_T;          /* CM_MotorCurrent (LINEAR) */

typedef uint16 VehSpeed_T;              /* CM_VehSpeed (LINEAR) */

typedef uint8 MotorSpeedCmd_T;          /* CM_MotorSpeedCmd (TEXTTABLE) */
#define MOTORSPEED_STOP         0u
#define MOTORSPEED_LOW          1u
#define MOTORSPEED_HIGH         2u

typedef uint8 WiperStatus_T;            /* CM_WiperStatus (TEXTTABLE) */
#define WIPERSTATUS_OFF         0u
#define WIPERSTATUS_INTERVAL    1u
#define WIPERSTATUS_AUTO        2u
#define WIPERSTATUS_LOW         3u
#define WIPERSTATUS_HIGH        4u
#define WIPERSTATUS_WASH        5u
#define WIPERSTATUS_FAULT       6u

typedef struct
{
    uint16 WashCycleCnt;
    uint8  BlockEventCnt;
    uint8  Reserved;
} WiperCtrl_NvBlockType;

/* ---------------------------------------------------------------------------
 * Service types (DEM, NvM)
 * ------------------------------------------------------------------------- */
typedef uint8 Dem_EventStatusType;
#define DEM_EVENT_STATUS_PASSED     ((Dem_EventStatusType)0x00u)
#define DEM_EVENT_STATUS_FAILED     ((Dem_EventStatusType)0x01u)
#define DEM_EVENT_STATUS_PREPASSED  ((Dem_EventStatusType)0x02u)
#define DEM_EVENT_STATUS_PREFAILED  ((Dem_EventStatusType)0x03u)

typedef uint8 NvM_RequestResultType;
#define NVM_REQ_OK                  ((NvM_RequestResultType)0u)
#define NVM_REQ_NOT_OK              ((NvM_RequestResultType)1u)
#define NVM_REQ_PENDING             ((NvM_RequestResultType)2u)
#define NVM_REQ_INTEGRITY_FAILED    ((NvM_RequestResultType)3u)
#define NVM_REQ_BLOCK_SKIPPED       ((NvM_RequestResultType)4u)
#define NVM_REQ_NV_INVALIDATED      ((NvM_RequestResultType)5u)
#define NVM_REQ_CANCELED            ((NvM_RequestResultType)6u)

#endif /* RTE_WIPERCTRL_TYPE_H */
