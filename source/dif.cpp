#include "../include/dif.h"


const char* difGetErrorMsg(DifError err)
{
    #define DEF_DIF_ERR(err, msg)                                             \
        case DIF_ERR_ ## err:                                                 \
            return msg;

    switch (err)
    {
        #include "../include/dif_err_codegen.inc"
        default:
            return "No such error was found";
    }
    #undef DEF_DIF_ERR
}

DifError difCtor(Dif* dif, FILE* logFile)
{
    if (dif == nullptr)
        return DIF_ERR_NULLPTR_PASSED;
    *dif = {};

    dif->logFile = logFile;
    LOG_FUNC_START(logFile);

    LOG_FUNC_END(logFile);
    return DIF_ERR_NO;
}
