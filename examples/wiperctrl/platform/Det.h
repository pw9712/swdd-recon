/**
 * \file   Det.h
 * \brief  Host stub of the Default Error Tracer API.
 */
#ifndef DET_H
#define DET_H

#include "Std_Types.h"

extern FUNC(Std_ReturnType, DET_CODE) Det_ReportError(uint16 ModuleId,
                                                      uint8  InstanceId,
                                                      uint8  ApiId,
                                                      uint8  ErrorId);

#endif /* DET_H */
