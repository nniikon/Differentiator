#include "../include/dif_operations.h"

#include "../include/dif.h"

double difEvl_add(TreeNode *node, double left, double right)
{
    return left + right;
}

TreeNode* difDif_add(Tree *tree, TreeNode *node)
{   
    treeElem_t elem = {};
    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = DIF_OPR_ADD;
    return treeCreateNode(tree, difDifNode_recursive(tree, node->leftBranch ),
                                difDifNode_recursive(tree, node->rightBranch),
                                node->parentBranch, elem);
}


double difEvl_sub(TreeNode *node, double left, double right)
{
    return left - right;
}

TreeNode* difDif_sub(Tree *tree, TreeNode *node)
{
    treeElem_t elem = {};
    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = DIF_OPR_SUB;
    return treeCreateNode(tree, difDifNode_recursive(tree, node->leftBranch ),
                                difDifNode_recursive(tree, node->rightBranch),
                                node->parentBranch, elem);
}


double difEvl_mul(TreeNode *node, double left, double right)
{
    return left * right;
}

TreeNode* difDif_mul(Tree *tree, TreeNode *node)
{
    treeElem_t elem = {};
    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = DIF_OPR_SUB;
    
    return treeCreateNode(tree, difDifNode_recursive(tree, node->leftBranch ),
                                difDifNode_recursive(tree, node->rightBranch),
                                node->parentBranch, elem);
}



double difEvl_div(TreeNode *node, double left, double right)
{
    return left / right;
}

TreeNode* difDif_div(Tree *tree, TreeNode *node)
{
}
