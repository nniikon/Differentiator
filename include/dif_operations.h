#ifndef DIF_OPERATIONS_H_
#define DIF_OPERATIONS_H_

#include "dif_tree_cfg.h"
#include "../binaryTree/include/tree.h"
#include "dif_hash.h"

#define DEF_DIF_OPR(opr, chr, ord)                                                 \
    double    difEvl_ ## opr (TreeNode* node, double left, double right);          \
    TreeNode* difDif_ ## opr (Tree* tree, TreeNode* node);                         \
    TreeNode* difSmp_ ## opr (Tree* tree, TreeNode* node);
#include "dif_operations_codegen.inc"
#undef DEF_DIF_OPR

struct DifOpr
{
    DifOprType  type;
    const char* name;
    long  hash;
    double    (*eval)(TreeNode* node, double left, double right);
    TreeNode* (*dif )(Tree* tree, TreeNode* node);
    TreeNode* (*smp )(Tree* tree, TreeNode* node);
    int order;
};

const DifOpr DIF_OPERATIONS[]
{
    #define DEF_DIF_OPR(opr, chr, ord)\
        {DIF_OPR_ ## opr, chr, difGetOprHash(chr), difEvl_ ## opr, difDif_ ## opr, difSmp_ ## opr, ord},
    #include "dif_operations_codegen.inc"
    #undef DEF_DIF_OPR
};

#endif