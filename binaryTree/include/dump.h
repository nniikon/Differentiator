#ifndef DUMP_H_
#define DUMP_H_

#include <stdio.h>
#include "../../common/logs/logs.h"

// Uses logs.h for backward compatabilty.

#ifndef DUMP_RELEASE
    #define DUMP(file ,...)         LOGF(file, __VA_ARGS__)
    #define DUMP_FUNC_START(file)   LOG_FUNC_START(file);
    #define DUMP_FUNC_SUCCESS(file) LOG_FUNC_END(file);
#else
    #define DUMP              do {} while (0)
    #define DUMP_FUNC_START   do {} while (0)
    #define DUMP_FUNC_SUCCESS do {} while (0)
#endif

#endif // DUMP_H_