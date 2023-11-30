#ifndef DIF_OPERATIONS_H_
#define DIF_OPERATIONS_H_

#include "dif_tree_cfg.h"
#include "../binaryTree/include/tree.h"

double    difEvl_add(TreeNode* node, double left, double right);
TreeNode* difDif_add(Tree* tree, TreeNode* node);

double    difEvl_sub(TreeNode* node, double left, double right);
TreeNode* difDif_sub(Tree* tree, TreeNode* node);

double    difEvl_mul(TreeNode* node, double left, double right);
TreeNode* difDif_mul(Tree* tree, TreeNode* node);

double    difEvl_div(TreeNode* node, double left, double right);
TreeNode* difDif_div(Tree* tree, TreeNode* node);

struct DifOpr
{
    DifOprType type;
    const char* name; 
    double    (*eval)(TreeNode* node, double left, double right);
    TreeNode* (*dif )(Tree* tree, TreeNode* node);
};

const DifOpr DIF_OPERATIONS[]
{
    {DIF_OPR_ADD, "+", difEvl_add, difDif_add},
    {DIF_OPR_SUB, "-", difEvl_sub, difDif_sub},
    {DIF_OPR_MUL, "*", difEvl_mul, difDif_mul},
    {DIF_OPR_DIV, "/", difEvl_div, difDif_div},
};


#endif