#include "logs.h"

#include <time.h>

const char timeMask[] = "%H:%M:%S";

void getCurrentTimeStr(char* str, size_t bufferSize)
{
    time_t timeInfo = time(NULL);
    tm* time = localtime(&timeInfo);

    strftime(str, bufferSize, timeMask, time);
}