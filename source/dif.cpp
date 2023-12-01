#include "../include/dif.h"

#include "../include/dif_operations.h"
#include "math.h"

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


static double difEvalNode_recursive(Tree* tree, TreeNode* node)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);

    if (node->data.type == DIF_NODE_TYPE_NUM)
    {
        LOGF(tree->debugInfo.dumpFile, "returning: %lg\n", node->data.value.num);
        return node->data.value.num;
    }
    double left  = difEvalNode_recursive(tree, node-> leftBranch);
    double right = difEvalNode_recursive(tree, node->rightBranch);

    double result = DIF_OPERATIONS[(int)node->data.value.opr].eval(node, left, right);

    LOGF(tree->debugInfo.dumpFile, "returning: %lg\n", result);
    LOG_FUNC_END(tree->debugInfo.dumpFile);
    return result;
}


DifError difEvalTree(Tree* tree, double* output)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);

    if (!tree)
        DIF_RETURN_LOG_ERROR(DIF_ERR_NULLPTR_PASSED, tree->debugInfo.dumpFile);

    *output = difEvalNode_recursive(tree ,tree->rootBranch);

    if (isnan(*output))
        DIF_RETURN_LOG_WARNING(DIF_ERR_DIVISION_BY_ZERO, tree->debugInfo.dumpFile);


    LOG_FUNC_END(tree->debugInfo.dumpFile);
    return DIF_ERR_NO;
}


TreeNode* difDifNode_recursive(Tree* tree, TreeNode* node)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    if (node->data.type == DIF_NODE_TYPE_NUM)
    {
        return treeCreateNode(tree, nullptr, nullptr, 
                        node->parentBranch, {0, DIF_NODE_TYPE_NUM});
    }

    if (node->data.type == DIF_NODE_TYPE_VAR)
    {
        return treeCreateNode(tree, nullptr, nullptr,
                        node->parentBranch, {1, DIF_NODE_TYPE_NUM});
    }

    LOGF(tree->debugInfo.dumpFile, "The operation is %s\n",
                                    DIF_OPERATIONS[(int)node->data.value.opr].name);
    LOG_FUNC_END(tree->debugInfo.dumpFile);
    return DIF_OPERATIONS[(int)node->data.value.opr].dif(tree, node);
}


DifError difDifTree(Tree* tree)
{
    if (!tree)
        DIF_RETURN_LOG_ERROR(DIF_ERR_NULLPTR_PASSED, nullptr); // QUESTION

    LOG_FUNC_START(tree->debugInfo.dumpFile);

    tree->rootBranch = difDifNode_recursive(tree, tree->rootBranch);
 
    LOG_FUNC_END(tree->debugInfo.dumpFile);
    return DIF_ERR_NO;
}