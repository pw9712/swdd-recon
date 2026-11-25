/**
 * \file   Platform_Types.h
 * \brief  Host stub of the AUTOSAR platform types (32-bit host, LSB first).
 *         Replace with the target platform header on real hardware.
 */
#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#define CPU_TYPE_8      8
#define CPU_TYPE_16     16
#define CPU_TYPE_32     32
#define CPU_TYPE        CPU_TYPE_32

#define MSB_FIRST       0
#define LSB_FIRST       1
#define CPU_BIT_ORDER   LSB_FIRST

#define HIGH_BYTE_FIRST 0
#define LOW_BYTE_FIRST  1
#define CPU_BYTE_ORDER  LOW_BYTE_FIRST

typedef unsigned char   boolean;
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef signed char     sint8;
typedef signed short    sint16;
typedef signed int      sint32;
typedef float           float32;
typedef double          float64;

#ifndef TRUE
#define TRUE  1u
#endif
#ifndef FALSE
#define FALSE 0u
#endif

#endif /* PLATFORM_TYPES_H */
