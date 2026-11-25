/**
 * \file   WiperCtrl_MemMap.h
 * \brief  Memory mapping for WiperCtrl (host stub).
 *
 * NOTE: No include guard by design - this header is included repeatedly,
 * once per START/STOP section define (AUTOSAR MemMap pattern).
 * On target, each branch would contain the compiler specific #pragma section.
 */

#define MEMMAP_ERROR

/* ---------------- code ---------------- */
#if defined(WiperCtrl_START_SEC_CODE)
    #undef WiperCtrl_START_SEC_CODE
    #undef MEMMAP_ERROR
#elif defined(WiperCtrl_STOP_SEC_CODE)
    #undef WiperCtrl_STOP_SEC_CODE
    #undef MEMMAP_ERROR

/* ---------------- variables, cleared ---------------- */
#elif defined(WiperCtrl_START_SEC_VAR_CLEARED_8)
    #undef WiperCtrl_START_SEC_VAR_CLEARED_8
    #undef MEMMAP_ERROR
#elif defined(WiperCtrl_STOP_SEC_VAR_CLEARED_8)
    #undef WiperCtrl_STOP_SEC_VAR_CLEARED_8
    #undef MEMMAP_ERROR
#elif defined(WiperCtrl_START_SEC_VAR_CLEARED_16)
    #undef WiperCtrl_START_SEC_VAR_CLEARED_16
    #undef MEMMAP_ERROR
#elif defined(WiperCtrl_STOP_SEC_VAR_CLEARED_16)
    #undef WiperCtrl_STOP_SEC_VAR_CLEARED_16
    #undef MEMMAP_ERROR
#elif defined(WiperCtrl_START_SEC_VAR_CLEARED_UNSPECIFIED)
    #undef WiperCtrl_START_SEC_VAR_CLEARED_UNSPECIFIED
    #undef MEMMAP_ERROR
#elif defined(WiperCtrl_STOP_SEC_VAR_CLEARED_UNSPECIFIED)
    #undef WiperCtrl_STOP_SEC_VAR_CLEARED_UNSPECIFIED
    #undef MEMMAP_ERROR

/* ---------------- constants ---------------- */
#elif defined(WiperCtrl_START_SEC_CONST_8)
    #undef WiperCtrl_START_SEC_CONST_8
    #undef MEMMAP_ERROR
#elif defined(WiperCtrl_STOP_SEC_CONST_8)
    #undef WiperCtrl_STOP_SEC_CONST_8
    #undef MEMMAP_ERROR
#elif defined(WiperCtrl_START_SEC_CONST_16)
    #undef WiperCtrl_START_SEC_CONST_16
    #undef MEMMAP_ERROR
#elif defined(WiperCtrl_STOP_SEC_CONST_16)
    #undef WiperCtrl_STOP_SEC_CONST_16
    #undef MEMMAP_ERROR
#endif

#if defined(MEMMAP_ERROR)
    #error "WiperCtrl_MemMap.h: wrong or missing section define"
#endif
