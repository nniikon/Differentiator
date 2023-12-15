#include "../include/dif_operations.h"

#include <math.h>
#include "../include/dif.h"
#include "../common/logs/logs.h"

const DifOpr* getDifOpr(TreeNode* node)
{
    return &DIF_OPERATIONS[(int)node->data.value.opr];
}

#define L node->leftBranch
#define R node->rightBranch
#define C node

// FIXME: ERROR CHECKS
#define CPY(node) treeCreateNode(tree, (node)->leftBranch, (node)->rightBranch, nullptr, (node)->data)

#define OPR(OPR, left, right) treeCreateNode(tree, (left), (right), nullptr,\
                                {{.opr = (DifOprType)DIF_OPR_##OPR}, DIF_NODE_TYPE_OPR})

#define DIF(node) OPR(DIF, CPY(node), nullptr)

#define NUM(NUM) treeCreateNode(tree, nullptr, nullptr, nullptr,\
                                {{.num = NUM}, DIF_NODE_TYPE_NUM})

/*FIXME error check*/
#define EVL(node) node = NUM(difEvalNode_recursive(tree, node, 0, 0));


static bool IS_CONST(TreeNode* node)
{
    difSetNodeConst(node);
    return node->data.isConst;
}

static bool IS_NUM(TreeNode* node, double num)
{
    return node != nullptr && (node->data.type == DIF_NODE_TYPE_NUM) 
                           && (fabs(node->data.value.num - num) < DIF_EPSILON);
}

static bool IS_ZERO(TreeNode* node)
{
    return IS_NUM(node, 0.0);
}

static bool IS_ONE(TreeNode* node)
{
    return IS_NUM(node, 1.0);
}

#define FUNC_EVL(name) double difEvl_ ## name (TreeNode* node, double left, double right)
#define FUNC_DIF(name) TreeNode* difDif_ ## name (Tree* tree, TreeNode* node)
#define FUNC_SMP(name) TreeNode* difSmp_ ## name (Tree* tree, TreeNode* node)


FUNC_EVL(ADD)
{
    return left + right;
}

FUNC_DIF(ADD)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    return OPR(ADD, DIF(L), DIF(R));
}

FUNC_SMP(ADD)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    if (IS_ZERO(L) && IS_ZERO(R))
        return NUM(0.0);
    if (IS_ZERO(L))
        return R;
    if (IS_ZERO(R))
        return L;
    return node;
}


FUNC_EVL(SUB)
{
    return left - right;
}

FUNC_DIF(SUB)
{
    return OPR(SUB, DIF(L), DIF(R));
}

FUNC_SMP(SUB)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    if (IS_ZERO(L) && IS_ZERO(R))
        return NUM(0.0);
    if (IS_ZERO(R))
        return L;
    return node;
}


FUNC_EVL(MUL)
{
    return left * right;
}

FUNC_DIF(MUL)
{
    return OPR(ADD, OPR(MUL, DIF(L), CPY(R)), 
                    OPR(MUL, CPY(L), DIF(R)));
}

FUNC_SMP(MUL)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    if (IS_ZERO(L) || IS_ZERO(R))
    {
        LOGF(tree->debugInfo.dumpFile, "%s\n", "multiplication by zero");
        return NUM(0.0);
    }
    if (IS_ONE(R))
        return L;
    if (IS_ONE(L))
        return R;
    LOG_FUNC_END(tree->debugInfo.dumpFile);
    return node;
}


FUNC_EVL(DIV)
{
    return left / right;
}

FUNC_DIF(DIV)
{
    return OPR(DIV, OPR(SUB, OPR(MUL, DIF(L), CPY(R)), OPR(MUL, DIF(R), CPY(L))),
                    OPR(MUL, CPY(R), CPY(R))); 
}

FUNC_SMP(DIV)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    if (IS_ZERO(L))
        return NUM(0.0);

    if (IS_ONE(R))
        return L;
    return node;
}


FUNC_EVL(LN)
{
    return log(left);
}

FUNC_DIF(LN)
{
    return OPR(DIV, DIF(L), CPY(L));
}

FUNC_SMP(LN)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    return node;
}


FUNC_EVL(LOG)
{
    return log10(left);
}

FUNC_DIF(LOG)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    return OPR(DIV, DIF(L),
                    OPR(MUL, OPR(LN, NUM(10), nullptr), CPY(L)));
}

FUNC_SMP(LOG)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    return node;
}


FUNC_EVL(POW)
{
    return pow(left, right);
}

FUNC_DIF(POW)
{
    if (IS_CONST(R))
    {
        EVL(R); 
        return OPR(MUL, NUM(R->data.value.num),
                        OPR(MUL, DIF(L),
                        OPR(POW, CPY(L), NUM(R->data.value.num - 1)))); 
    }
    return OPR(MUL, OPR(POW, CPY(L), CPY(R)), 
                    DIF(OPR(MUL, OPR(LN, CPY(L), nullptr), CPY(R))));
}

FUNC_SMP(POW)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    if (IS_ZERO(L))
        return NUM(0.0);
    if (IS_ZERO(R))
        return NUM(1.0);
    if (IS_ONE(R))
        return L;
    if (IS_ONE(L))
        return NUM(1.0);
    return node;
}


FUNC_EVL(SIN)
{
    return sin(left);
}

FUNC_DIF(SIN)
{
    return OPR(MUL, OPR(COS, CPY(L), nullptr),
                    DIF(L));
}

FUNC_SMP(SIN)
{
    return node;
}


FUNC_EVL(COS)
{
    return cos(left);
}

FUNC_DIF(COS)
{
    return OPR(MUL, NUM(-1.0),
                    OPR(MUL, OPR(SIN, CPY(L), nullptr), DIF(L)));
}

FUNC_SMP(COS)
{
    return node;
}


FUNC_EVL(TAN)
{
    return tan(left);
}

FUNC_DIF(TAN)
{
    return OPR(DIV, DIF(L),
                    OPR(POW, OPR(COS, CPY(L), nullptr), NUM(2.0)));
}

FUNC_SMP(TAN)
{
    return node;
}


FUNC_EVL(DIF)
{
    return NAN;
}

FUNC_DIF(DIF)
{
    return node;
}

FUNC_SMP(DIF)
{
    return node;
}