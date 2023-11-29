#ifndef TREE_H_
#define TREE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../common/memAllocations/memAllocations.h"
#include "../../include/dif_tree_cfg.h"

typedef struct TreeNode
{
    treeElem_t data;
    struct TreeNode* leftBranch;
    struct TreeNode* rightBranch;
    struct TreeNode* parentBranch;
} TreeNode;

typedef struct
{
	int isDebug;
	FILE* dumpFile;
	unsigned int dumpIndex;
} TreeDebugInfo;

typedef struct
{
    TreeNode* rootBranch;
    DynArr memBuffer;
    unsigned int size;
	TreeDebugInfo debugInfo;
} Tree;

typedef enum
{
    #define DEF_TREE_ERR(err, msg) TREE_ERROR_ ## err,
    #include "tree_err_codegen.inc"
    #undef  DEF_TREE_ERR
} TreeError;

TreeError treeCtor(Tree* tree, FILE* dumpFile);
TreeError treeDtor(Tree* tree);

TreeError treeInsertLeft (Tree* tree, TreeNode* node, treeElem_t data);
TreeError treeInsertRight(Tree* tree, TreeNode* node, treeElem_t data);

TreeError treeVerify(Tree* tree);

const char* treeGetErrorMsg(TreeError err);

TreeNode* treeCreateNode(Tree* tree, TreeNode* left, TreeNode* right,
                                     TreeNode* parent, treeElem_t data);
TreeNode* treeCreateEmptyNode(Tree* tree);

TreeError treeCpy(const Tree* src, Tree* dst);

#endif // TREE_H_
