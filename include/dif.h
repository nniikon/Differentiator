#ifndef DIF_H_
#define DIF_H_

#include "../binaryTree/include/tree.h"
#include "../common/logs/logs.h"
#include "dif_cfg.h"

struct Dif
{
    Tree* tree;
    FILE* logFile;
    int dumpIndex;
};

enum DifError
{
    #define DEF_DIF_ERR(err, msg) DIF_ERR_ ## err,
    #include "dif_err_codegen.inc"
    #undef  DEF_DIF_ERR
};

DifError difCtor(Dif* dif, FILE* logFile);

const char* difGetErrorMsg(DifError err);

#define DIF_RETURN_LOG(err, dif)                                              \
    do {                                                                      \
        LOG_FUNC_END(dif->logFile);                                           \
        return err;                                                           \
    } while (0)

#define DIF_RETURN_LOG_ERROR(err, dif)                                        \
    do {                                                                      \
        LOGF_ERR(dif->logFile, "%s", difGetErrorMsg(err));                    \
        DIF_RETURN_LOG(err, dif);                                             \
    } while (0)

#define DIF_RETURN_LOG_WARNING(err, dif)                                      \
    do {                                                                      \
        LOGF_WARNING(dif->logFile, "%s", difGetErrorMsg(err));                \
        DIF_RETURN_LOG(err, dif);                                             \
    } while (0)

#endif