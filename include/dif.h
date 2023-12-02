#ifndef DIF_H_
#define DIF_H_

#include "../binaryTree/include/tree.h"
#include "../common/logs/logs.h"
#include "dif_cfg.h"

struct Dif
{
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

DifError difEvalTree(Tree* tree, double* output);
DifError difDifTree (Tree* tree);

TreeNode* difDifNode_recursive(Tree* tree, TreeNode* node);

bool difSimplifyConsts      (Tree* tree);
bool difSimplifyNeutralElems(Tree* tree);
void difSimplify            (Tree* tree);

#define DIF_RETURN_LOG(err, logFile)                                          \
    do {                                                                      \
        LOG_FUNC_END(logFile);                                                \
        return err;                                                           \
    } while (0)

#define DIF_RETURN_LOG_ERROR(err, logFile)                                    \
    do {                                                                      \
        LOGF_ERR(logFile, "%s", difGetErrorMsg(err));                         \
        DIF_RETURN_LOG(err, logFile);                                         \
    } while (0)

#define DIF_RETURN_LOG_WARNING(err, logFile)                                  \
    do {                                                                      \
        LOGF_WRN(logFile, "%s", difGetErrorMsg(err));                         \
        DIF_RETURN_LOG(err, logFile);                                         \
    } while (0)

#endif