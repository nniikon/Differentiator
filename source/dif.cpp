#include "../include/dif.h"

#include <assert.h>
#include <math.h>
#include "../include/dif_operations.h"
#include "../include/dif_graphDump.h"
#include "../treeParser/treeParser.h"
#include "../include/dif_latexLogs.h"

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


DifError difCtor(Dif* dif, FILE* logFile, FILE* outputFile)
{
    if (!dif)
        DIF_RETURN_LOG_ERROR(DIF_ERR_NULLPTR_PASSED, logFile);
    *dif = {};

    dif->logFile = logFile;
    dif->outputFile = outputFile;
    LOG_FUNC_START(logFile);

    LOG_FUNC_END(logFile);
    return DIF_ERR_NO;
}


double difEvalNode_recursive(Tree* tree, TreeNode* node, bool hasVars, double varValue)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);

    if (hasVars && node->data.type == DIF_NODE_TYPE_VAR)
    {
        return varValue;
    }

    if (node->data.type == DIF_NODE_TYPE_NUM)
    {
        LOGF(tree->debugInfo.dumpFile, "returning: %lg\n", node->data.value.num);
        return node->data.value.num;
    }

    double left = difEvalNode_recursive(tree, node->leftBranch, hasVars, varValue);

    double right = NAN;
    if (node->rightBranch)
        right = difEvalNode_recursive(tree, node->rightBranch, hasVars, varValue);

    double result = getDifOpr(node)->eval(node, left, right);

    LOGF(tree->debugInfo.dumpFile, "returning: %lg\n", result);
    LOG_FUNC_END(tree->debugInfo.dumpFile);
    return result;
}


DifError difEvalTree(Tree* tree, double* output)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);

    if (!tree)
        DIF_RETURN_LOG_ERROR(DIF_ERR_NULLPTR_PASSED, tree->debugInfo.dumpFile);

    *output = difEvalNode_recursive(tree, tree->rootBranch, 0, NAN);

    if (isnan(*output))
        DIF_RETURN_LOG_WARNING(DIF_ERR_DIVISION_BY_ZERO, tree->debugInfo.dumpFile);

    LOG_FUNC_END(tree->debugInfo.dumpFile);
    return DIF_ERR_NO;
}


DifError difDumpLatex(Dif* dif, Tree* tree)
{
    DifError err = DIF_ERR_NO;

    err = difGraphDump(dif, tree);
    if (err) return err;

    difLatexDumpTree(dif, tree);

    return err;
} 


static void difDifNode_recursive(Dif* dif, Tree* tree, TreeNode** node, bool isLoud)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    if (*node == nullptr)
        return;

    if ((*node)->data.type == DIF_NODE_TYPE_OPR && 
        (*node)->data.value.opr == DIF_OPR_DIF)
    {

        switch ((*node)->leftBranch->data.type)
        {
            case DIF_NODE_TYPE_NUM:
                *node = treeCreateNode(tree, nullptr, nullptr, nullptr,
                                   {{.num = 0.0}, DIF_NODE_TYPE_NUM});
                break;
            case DIF_NODE_TYPE_VAR:
                *node = treeCreateNode(tree, nullptr, nullptr, nullptr,
                                  {{.num = 1.0}, DIF_NODE_TYPE_NUM});
                break;
            case DIF_NODE_TYPE_OPR:
                LOGF(tree->debugInfo.dumpFile, "%s", "Differentiating an operation\n");
                *node = getDifOpr((*node)->leftBranch)->dif(tree, (*node)->leftBranch);
                if (isLoud)
                {
                    difSimplify (     tree);
                    difDumpLatex(dif, tree);
                }

                difDifNode_recursive(dif, tree, &(*node)->leftBranch,  isLoud);
                difDifNode_recursive(dif, tree, &(*node)->rightBranch, isLoud);
                return;
            default:
                assert(0);
                *node =  treeCreateNode(tree, nullptr, nullptr, nullptr,
                                    {{.num = NAN}, DIF_NODE_TYPE_NUM});
        }
    }
    else
    {
        difDifNode_recursive(dif, tree, &(*node)->leftBranch,  isLoud);
        difDifNode_recursive(dif, tree, &(*node)->rightBranch, isLoud);
        return;
    }
}


static DifError difDifTree_internal(Dif* dif, Tree* tree, bool isLoud)
{
    if (!tree)
        DIF_RETURN_LOG_ERROR(DIF_ERR_NULLPTR_PASSED, nullptr); // QUESTION

    LOG_FUNC_START(tree->debugInfo.dumpFile);

    difDifNode_recursive(dif, tree, &tree->rootBranch, isLoud);
 
    LOG_FUNC_END(tree->debugInfo.dumpFile);
    return DIF_ERR_NO;
}


DifError difDifTreeDump(Dif* dif, Tree* tree)
{
    DifError err = difDumpLatex(dif, tree);
    if (err) return err;

    err = difDifTree_internal(dif, tree, true);
    if (err) return err;

    err = difSimplify(tree);
    if (err) return err;

    err = difDumpLatex(dif, tree);
    if (err) return err;

    return err;
}


DifError difDifTree(Dif* dif, Tree* tree)
{
    return difDifTree_internal(dif, tree, false);
}


bool difSetNodeConst(TreeNode* node)
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
            if (node->data.value.opr == DIF_OPR_DIF)
            {
                node->data.isConst = false;
                break;
            }
            node->data.isConst = difSetNodeConst(node->leftBranch ) && 
                                 difSetNodeConst(node->rightBranch);
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
        LOGF_COLOR(tree->debugInfo.dumpFile, green, "Evaluated %s\n", getDifOpr(node)->name);
        node->data.value.num = difEvalNode_recursive(tree, node, 0, NAN);
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
    difSetNodeConst(tree->rootBranch);
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
        *node = getDifOpr(oldNode)->smp(tree, oldNode);
        if (oldNode != *node)
        {
            LOGF_COLOR(tree->debugInfo.dumpFile, green, "Simplified %s\n", getDifOpr(oldNode)->name);
            return true;
        }
    }

    return difSimplifyNeutralElems_recursive(tree, &(*node)-> leftBranch) ||
           difSimplifyNeutralElems_recursive(tree, &(*node)->rightBranch);


    LOG_FUNC_END(tree->debugInfo.dumpFile);
}


bool difSimplifyNeutralElems(Tree* tree)
{
    return difSimplifyNeutralElems_recursive(tree, &tree->rootBranch);
}


DifError difSimplify(Tree* tree)
{
    while (difSimplifyNeutralElems(tree) ||
           difSimplifyConsts      (tree))
        ;
    return DIF_ERR_NO;
}


bool difIsDoubleZero(double num)
{
    return fabs(num) < DIF_EPSILON;
}