#include "../include/dif_operations.h"

#include "../include/dif.h"

#define L node->leftBranch
#define R node->rightBranch
#define P node->parentBranch
#define C node

#define DIF(node) difDifNode_recursive(tree, (node))
#define CPY(node) treeCreateNode(tree, (node)->leftBranch, (node)->rightBranch, (node)->parentBranch, (node)->data)
#define OPR(OPR, left, right, parent) treeCreateNode(tree, (left), (right), (parent), {{(DifOprType)DIF_OPR_##OPR}, DIF_NODE_TYPE_OPR})

double difEvl_add(TreeNode *node, double left, double right)
{
    return left + right;
}

TreeNode *difDif_add(Tree *tree, TreeNode *node)
{
    return OPR(ADD, DIF(L), DIF(R), P);
}

double difEvl_sub(TreeNode *node, double left, double right)
{
    return left - right;
}

TreeNode *difDif_sub(Tree *tree, TreeNode *node)
{
    return OPR(SUB, DIF(L), DIF(R), P);
}

double difEvl_mul(TreeNode *node, double left, double right)
{
    return left * right;
}

TreeNode *difDif_mul(Tree *tree, TreeNode *node)
{
    return OPR(ADD, OPR(MUL, DIF(L), CPY(R), C), OPR(MUL, CPY(L), DIF(R), C), P);
}

double difEvl_div(TreeNode *node, double left, double right)
{
    return left / right;
}

TreeNode *difDif_div(Tree *tree, TreeNode *node)
{
    return OPR(DIV, OPR(SUB, OPR(MUL, DIF(L), CPY(R), nullptr), OPR(MUL, DIF(R), CPY(L), nullptr), nullptr), OPR(MUL, CPY(R), CPY(R), nullptr), nullptr); 
}
