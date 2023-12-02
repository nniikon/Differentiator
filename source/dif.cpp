#include "../include/dif.h"

#include <assert.h>
#include <math.h>
#include "../include/dif_operations.h"

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
                              nullptr, {0, DIF_NODE_TYPE_NUM});
    }

    if (node->data.type == DIF_NODE_TYPE_VAR)
    {
        return treeCreateNode(tree, nullptr, nullptr,
                              nullptr, {1, DIF_NODE_TYPE_NUM});
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


static bool difSetNodeConst(Tree* tree, TreeNode* node)
{
    if (!node)
        return true;

    switch(node->data.type)
    {
        case DIF_NODE_TYPE_NUM:
            node->data.isConst = true;
            break;
        case DIF_NODE_TYPE_VAR:
            node->data.isConst = false;
            break;
        case DIF_NODE_TYPE_OPR:
            node->data.isConst = difSetNodeConst(tree, node->leftBranch ) && 
                                 difSetNodeConst(tree, node->rightBranch);
            break;
        default:
            //fprintf(stderr, "MY FUCKING TYPE IS : %d\n", (int)node->data.type);
            assert(0);
            return false;
    }
    return node->data.isConst;
}


static bool difSimplifyConsts_recursive(Tree* tree, TreeNode* node)
{
    if (!node)
        return false;
    if (node->data.isConst && node->data.type == DIF_NODE_TYPE_OPR)
    {
        node->data.value.num = difEvalNode_recursive(tree, node);
        node->data.type = DIF_NODE_TYPE_NUM;
        node->leftBranch  = nullptr;
        node->rightBranch = nullptr;
        return true;
    }
    else
    {
        return difSimplifyConsts_recursive(tree, node-> leftBranch) || 
               difSimplifyConsts_recursive(tree, node->rightBranch);
    }
}


bool difSimplifyConsts(Tree* tree)
{
    difSetNodeConst(tree, tree->rootBranch);
    return difSimplifyConsts_recursive(tree, tree->rootBranch);
}


static bool difSimplifyNeutralElems_recursive(Tree* tree, TreeNode** node)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    LOGF(tree->debugInfo.dumpFile, "node: %p\n", node);
    if (*node == nullptr)
        return false;

    if ((*node)->data.type == DIF_NODE_TYPE_OPR)
    {
        TreeNode* oldNode = *node;
        *node = DIF_OPERATIONS[(int)(*node)->data.value.opr].smp(tree, (*node));
        if (oldNode != *node)
            return true;
    }

    return difSimplifyNeutralElems_recursive(tree, &(*node)-> leftBranch) ||
           difSimplifyNeutralElems_recursive(tree, &(*node)->rightBranch);


    LOG_FUNC_END(tree->debugInfo.dumpFile);
}


bool difSimplifyNeutralElems(Tree* tree)
{
    return difSimplifyNeutralElems_recursive(tree, &tree->rootBranch);
}


void difSimplify(Tree* tree)
{
    while (difSimplifyNeutralElems(tree) ||
           difSimplifyConsts      (tree))
        ;
    
}