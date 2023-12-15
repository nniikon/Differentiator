#ifndef LOGS_H_
#define LOGS_H_

#include <stdio.h> 
#include <stdlib.h>

void getCurrentTimeStr(char* str, size_t bufferSize);

#ifndef NLOG
    #define LOG_START_COLOR(file, color)                                         \
    do                                                                           \
    {                                                                            \
        if (file != NULL)                                                        \
        {                                                                        \
            fprintf(file, "<font color=" #color ">");                            \
        }                                                                        \
    } while (0)

    #define LOG_COLOR(file, color, str, ...)                                     \
    do                                                                           \
    {                                                                            \
        if (file != NULL)                                                        \
        {                                                                        \
            LOG_START_COLOR(file, color);                                        \
            fprintf(file, str, __VA_ARGS__);                                     \
        }                                                                        \
    } while (0)

    #define LOG_END(file)                                                        \
    do                                                                           \
    {                                                                            \
        if (file != NULL)                                                        \
        {                                                                        \
            fprintf(file, "</font>");                                            \
        }                                                                        \
    } while (0)

    #define LOGF_COLOR(file, color, str, ...)                                    \
    do                                                                           \
    {                                                                            \
        const int timeBufferSize_log = 10;                                       \
        const int     bufferSize_log = 39;                                       \
        if (file != NULL)                                                        \
        {                                                                        \
            char     buffer_log[    bufferSize_log] = {};                        \
            char timeBuffer_log[timeBufferSize_log] = {};                        \
            getCurrentTimeStr(timeBuffer_log, bufferSize_log);                   \
            snprintf(buffer_log, bufferSize_log, "[%s]{%s(%-3d)}: ",             \
                            timeBuffer_log, __FILE__, __LINE__);                 \
            LOG_COLOR(file, lightgreen, "%-*s",                                  \
                                bufferSize_log, buffer_log);                     \
            LOG_COLOR(file, color, str, __VA_ARGS__);                            \
            LOG_END(file);                                                       \
        }                                                                        \
    } while (0)

    #define LOGF(file, str, ...)     LOGF_COLOR(file, white,  "\t"        str, __VA_ARGS__)
    #define LOGF_ERR(file, str, ...) LOGF_COLOR(file, red,    "ERROR! "   str, __VA_ARGS__)
    #define LOGF_WRN(file, str, ...) LOGF_COLOR(file, orange, "WARNING! " str, __VA_ARGS__)

    #define LOG_FUNC_START(file) LOGF_COLOR(file, purple, "%s started\n", __PRETTY_FUNCTION__)
    #define LOG_FUNC_END(file)   LOGF_COLOR(file, purple, "%s ended\n",   __PRETTY_FUNCTION__)
#else
    #define LOG_COLOR(...)           (void)0
    #define LOGF(...)                (void)0
    #define LOGF_ERR(file, str, ...) (void)0 
    #define LOGF_ERR(file, str, ...) (void)0 
    #define LOGF_WRN(file, str, ...) (void)0
    #define LOG_FUNC_START(file)     (void)0
    #define LOG_FUNC_END(file)       (void)0

#endif

#endif // LOGS_H_