/**
 * \file   Std_Types.h
 * \brief  Host stub of the AUTOSAR standard types.
 */
#ifndef STD_TYPES_H
#define STD_TYPES_H

#include "Platform_Types.h"
#include "Compiler.h"

typedef uint8 Std_ReturnType;

#ifndef STATUSTYPEDEFINED
#define STATUSTYPEDEFINED
#define E_OK        0u
#endif
#define E_NOT_OK    1u

#define STD_HIGH    1u
#define STD_LOW     0u
#define STD_ACTIVE  1u
#define STD_IDLE    0u
#define STD_ON      1u
#define STD_OFF     0u

typedef struct
{
    uint16 vendorID;
    uint16 moduleID;
    uint8  sw_major_version;
    uint8  sw_minor_version;
    uint8  sw_patch_version;
} Std_VersionInfoType;

#endif /* STD_TYPES_H */
