#include "../common/logs/logs.h"
#include "../binaryTree/include/tree.h"
#include "../include/dif.h"
#include "../include/dif_graphDump.h"

void createTestTree(Tree* tree, FILE* dumpFile);

int main()
{
    FILE* file = fopen("logs.html", "w");
    if (file == nullptr)
        return -1;
    fprintf(file, "<pre style=\"background: #000000;color:#000000;\">");
    setvbuf(file, NULL, _IONBF, 0);

    Dif dif = {};
    difCtor(&dif, file);

    Tree tree = {};
    createTestTree(&tree, file);

    difGraphDump(&dif, &tree);

    difDifTree  (&tree);
    difGraphDump(&dif, &tree);

    treeDtor(&tree);

    fclose(file);
}

DifVar var =
    {.name = "x",
     .value = 1.0 };


void createTestTree(Tree* tree, FILE* dumpFile)
{
    treeCtor(tree, dumpFile);

    treeElem_t elem = {};
    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = DIF_OPR_DIV;

    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = DIF_OPR_MUL;
    tree->rootBranch->data = elem;

    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = DIF_OPR_ADD;
    treeInsertRight(tree, tree->rootBranch, elem);

    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = DIF_OPR_SUB;
    treeInsertLeft (tree, tree->rootBranch, elem);

    elem.type = DIF_NODE_TYPE_NUM;
    elem.value.num = 2000;
    treeInsertLeft (tree, tree->rootBranch->leftBranch, elem);
    elem.value.num = 1000;
    treeInsertRight(tree, tree->rootBranch->leftBranch, elem);

    elem.value.num = 3;
    treeInsertLeft (tree, tree->rootBranch->rightBranch, elem);
    elem.type = DIF_NODE_TYPE_VAR;
    elem.value.var = &var;
    treeInsertRight(tree, tree->rootBranch->rightBranch, elem);

}