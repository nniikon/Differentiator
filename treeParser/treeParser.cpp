#include "treeParser.h"

#include "assert.h"
#include "../include/dif_operations.h"
#include "../common/logs/logs.h"

static void parserPutNodeNameToFile(Tree* tree, TreeNode* node, FILE* file)
{
    switch(node->data.type)
    {
        case DIF_NODE_TYPE_NUM:
            fprintf(file, "%lg", node->data.value.num);
            break;
        case DIF_NODE_TYPE_OPR:
            fprintf(file, "%s", DIF_OPERATIONS[(int)node->data.value.opr].name);
            break;
        case DIF_NODE_TYPE_VAR:
            fprintf(file, "%s", node->data.value.var->name); 
            break;
        default:
            LOGF_ERR(tree->debugInfo.dumpFile, "%s\n", "Unknown node type");
            assert(0);
            return;
    }
}


static void parserPutNodeToFile_recursive(Tree* tree, TreeNode* node, FILE* file)
{
    if (!node)
        return;

    if (node->data.type == DIF_NODE_TYPE_OPR)
        fprintf(file, "(");

    parserPutNodeToFile_recursive(tree, node->leftBranch , file);
    parserPutNodeNameToFile      (tree, node             , file);
    parserPutNodeToFile_recursive(tree, node->rightBranch, file);

    if (node->data.type == DIF_NODE_TYPE_OPR)
        fprintf(file, ")");
}


void parserPutTreeToFile(Tree* tree, FILE* file)
{
    assert(tree);
    if (!tree)
        return;
    
    parserPutNodeToFile_recursive(tree, tree->rootBranch, file);
}