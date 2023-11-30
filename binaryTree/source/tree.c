#include "../include/tree.h"

#include <assert.h>
#include "../include/dump.h"
#include "../include/tree_fileOperations.h"

#define DUMP_RETURN_ERROR(err)                                                \
    do                                                                        \
    {                                                                         \
        DUMP(tree->debugInfo.dumpFile, "%s\n", treeGetErrorMsg(err));         \
        return err;                                                           \
    } while (0)

#ifndef TREE_RELEASE
    #define TREE_VERIFY_OR_ELSE(tree)                                         \
        do                                                                    \
        {                                                                     \
            TreeError err = treeVerify(tree);                                 \
            if (err != TREE_ERROR_NO)                                         \
                DUMP_RETURN_ERROR(err);                                       \
        } while (0)
#else
    #define TREE_VERIFY_OR_ELSE(tree) do {} while(0)
#endif


typedef enum TreePrintOrder
{
    TREE_ORDER_INORDER,
    TREE_ORDER_PREORDER,
    TREE_ORDER_POSTORDER,
} TreePrintOrder;


typedef enum TreeDirection
{
    TREE_DIR_RIGHT,
    TREE_DIR_LEFT,
} TreeDirection;


static TreeError treeInsert(Tree* tree, TreeNode* node, TreeDirection dir, treeElem_t data);

static unsigned int treeCalcSize(Tree* tree);

static unsigned int treeCalcSize_recursive(TreeNode* node);

static TreeNode* treeCalloc(Tree* tree);

const char* treeGetErrorMsg(TreeError err)
{
    #define DEF_TREE_ERR(err, msg)                                            \
        case TREE_ERROR_ ## err:                                              \
            return msg;                                                       \

    switch (err)
    {
        #include "../include/tree_err_codegen.inc"
        default:
            return "No such error was found";
    }
    #undef DEF_TREE_ERR
}


TreeError treeCtor(Tree* tree, FILE* dumpFile)
{
	if (tree == NULL)
		return TREE_ERROR_NULLPTR_PASSED;
	memset(tree, 0, sizeof(Tree));

	if (dumpFile != NULL)
	{
		tree->debugInfo.dumpIndex = 0;
	}
	else
	{
		tree->debugInfo.dumpIndex = (unsigned int) -1;
	}
	tree->debugInfo.dumpFile = NULL;


    int dynArrErr = dynArrCtor(&tree->memBuffer, sizeof(TreeNode));
    if (dynArrErr == -1)
    {
        DUMP_RETURN_ERROR(TREE_ERROR_BAD_MEM_ALLOC);
    }

    tree->rootBranch = treeCalloc(tree);
    if (tree->rootBranch == NULL)
    {
        DUMP_RETURN_ERROR(TREE_ERROR_BAD_MEM_ALLOC);
    }
    tree->size++;

    DUMP_FUNC_SUCCESS(tree->debugInfo.dumpFile);
    return TREE_ERROR_NO;
}


TreeError treeDtor(Tree* tree)
{
    DUMP_FUNC_START(tree->debugInfo.dumpFile);

    dynArrDtor(&tree->memBuffer);
    memset(tree, 0, sizeof(Tree));

    DUMP_FUNC_SUCCESS(tree->debugInfo.dumpFile);
    return TREE_ERROR_NO;
}


inline static TreeNode* treeCalloc(Tree* tree)
{
    return (TreeNode*) dynArrCalloc(&tree->memBuffer);
}


TreeError treeVerify(Tree* tree)
{
    if (tree == NULL)
        DUMP_RETURN_ERROR(TREE_ERROR_NULLPTR_PASSED);

    if (tree->rootBranch == NULL)
        DUMP_RETURN_ERROR(TREE_ERROR_NO_CTOR);

    if (tree->memBuffer.buffer == NULL)
        DUMP_RETURN_ERROR(TREE_ERROR_NO_CTOR);

    if (tree->size != treeCalcSize(tree))
        DUMP_RETURN_ERROR(TREE_ERROR_UNEXPECTED_SIZE);

    return TREE_ERROR_NO;
}


static unsigned int treeCalcSize_recursive(TreeNode* node)
{
    unsigned int addSize = 1;

    if (node->rightBranch != NULL)
        addSize += treeCalcSize_recursive(node->rightBranch);
    if (node-> leftBranch != NULL)
        addSize += treeCalcSize_recursive(node-> leftBranch);

    return addSize;
}


static unsigned int treeCalcSize(Tree* tree)
{
    return treeCalcSize_recursive(tree->rootBranch);
}


static TreeError treeInsert(Tree* tree, TreeNode* node, TreeDirection dir, treeElem_t data)
{
    TREE_VERIFY_OR_ELSE(tree);
    if (node == NULL)
        DUMP_RETURN_ERROR(TREE_ERROR_NULLPTR_PASSED);

    TreeNode* newNode = treeCalloc(tree);
    if (newNode == NULL)
        DUMP_RETURN_ERROR(TREE_ERROR_BAD_MEM_ALLOC);
    newNode->data = data;
    tree->size++;

    switch(dir)
    {
        case TREE_DIR_RIGHT:
            node->rightBranch = newNode;
            break;
        case TREE_DIR_LEFT:
            node->leftBranch  = newNode;
            break;
        default:
            assert(0);
            return TREE_ERROR_UNEXPECTED;
    }
    return TREE_ERROR_NO;
}


TreeError treeInsertRight(Tree* tree, TreeNode* node, treeElem_t data)
{
    return treeInsert(tree, node, TREE_DIR_RIGHT, data);
}


TreeError treeInsertLeft(Tree* tree, TreeNode* node, treeElem_t data)
{
    return treeInsert(tree, node, TREE_DIR_LEFT , data);
}


TreeNode* treeCreateNode(Tree* tree, TreeNode* left, TreeNode* right, TreeNode* parent, treeElem_t data)
{
    TreeNode* node = treeCalloc(tree);
    if (node == NULL)
        return NULL;

    tree->size++;

    node->data         = data;
    node->leftBranch   = left;
    node->rightBranch  = right;
    node->parentBranch = parent;

    return node;
}


TreeNode* treeCreateEmptyNode(Tree* tree)
{
    TreeNode* node = treeCalloc(tree);
    if (node != NULL)
        tree->size++;
    return node;
}


static TreeError treeCpy_recursive(Tree* dstTree, const TreeNode* src,
                                                        TreeNode** dst)
{
    if (src == NULL)
        return TREE_ERROR_NO;

    *dst = treeCreateEmptyNode(dstTree);
    if (*dst == NULL)
        return TREE_ERROR_BAD_MEM_ALLOC;
    (*dst)->data = src->data;

    TreeError
    err = treeCpy_recursive(dstTree, src->leftBranch,  &((*dst)->leftBranch));
    if (err != TREE_ERROR_NO) return err;
    err = treeCpy_recursive(dstTree, src->rightBranch, &((*dst)->rightBranch));
    if (err != TREE_ERROR_NO) return err;

    return TREE_ERROR_NO;
}


TreeError treeCpy(const Tree* tree, Tree* dst)
{
    if (tree == NULL)
        DUMP_RETURN_ERROR(TREE_ERROR_NULLPTR_PASSED);

   TreeError err = treeCtor(dst, tree->debugInfo.dumpFile);
   if (err != TREE_ERROR_NO)
        return err;

    err = treeCpy_recursive(dst, tree->rootBranch, &dst->rootBranch);
    if (err != TREE_ERROR_NO)
        DUMP_RETURN_ERROR(err);

    return TREE_ERROR_NO;
}



















