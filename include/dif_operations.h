#ifndef DIF_OPERATIONS_H_
#define DIF_OPERATIONS_H_

#include "dif_tree_cfg.h"
#include "../binaryTree/include/tree.h"

#define DEF_DIF_OPR(opr, chr)                                                 \
    double    difEvl_ ## opr (TreeNode* node, double left, double right);          \
    TreeNode* difDif_ ## opr (Tree* tree, TreeNode* node);                         \
    TreeNode* difSmp_ ## opr (Tree* tree, TreeNode* node);
#include "dif_operations_codegen.inc"
#undef DEF_DIF_OPR

struct DifOpr
{
    DifOprType type;
    const char* name; 
    double    (*eval)(TreeNode* node, double left, double right);
    TreeNode* (*dif )(Tree* tree, TreeNode* node);
    TreeNode* (*smp )(Tree* tree, TreeNode* node);
};

const DifOpr DIF_OPERATIONS[]
{
    #define DEF_DIF_OPR(opr, chr) {DIF_OPR_ ## opr, chr, difEvl_ ## opr, difDif_ ## opr, difSmp_ ## opr},
    #include "dif_operations_codegen.inc"
    #undef  DEF_DIF_OPR
};


#endif