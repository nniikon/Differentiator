#ifndef DIF_OPERATIONS_H_
#define DIF_OPERATIONS_H_

#include "dif_tree_cfg.h"
#include "../binaryTree/include/tree.h"
// #include "dif_hash.h"

#define DEF_DIF_OPR(opr, ...)                                                      \
    double    difEvl_ ## opr (TreeNode* node, double left, double right);          \
    TreeNode* difDif_ ## opr (Tree* tree, TreeNode* node);                         \
    TreeNode* difSmp_ ## opr (Tree* tree, TreeNode* node);
#include "dif_operations_codegen.inc"
#undef DEF_DIF_OPR

struct DifOpr
{
    DifOprType  type;
    const char* name;
    int size;
    double    (*eval)(TreeNode* node, double left, double right);
    TreeNode* (*dif )(Tree* tree, TreeNode* node);
    TreeNode* (*smp )(Tree* tree, TreeNode* node);
    bool isBinary;
    bool isFunctionLike;
    int order;
};

const DifOpr DIF_OPERATIONS[]
{
    #define DEF_DIF_OPR(opr, chr, ord, isBin, isFunc)\
        {DIF_OPR_ ## opr, chr, sizeof(chr) - 1, difEvl_ ## opr, difDif_ ## opr, difSmp_ ## opr, isBin, isFunc, ord},
    #include "dif_operations_codegen.inc"
    #undef DEF_DIF_OPR

    {DIF_OPR_END, "END", 3, nullptr, nullptr, nullptr, 0, 0, -1},
};

const int DIF_N_OPERATIONS = sizeof(DIF_OPERATIONS) / sizeof(DIF_OPERATIONS[0]);

const DifOpr* getDifOpr(TreeNode* node);

#endif