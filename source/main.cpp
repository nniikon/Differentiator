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
    fprintf(file, "<pre>");
    setvbuf(file, NULL, _IONBF, 0);

    Dif dif = {};
    Tree tree = {};
    difCtor(&dif, file);

    createTestTree(&tree, file);

    difGraphDump(&dif, &tree);

    Tree dst = {};
    treeCpy(&tree, &dst);

    difGraphDump(&dif, &dst);
    treeDtor(&tree);
    treeDtor(&dst);

    fclose(file);
}


void createTestTree(Tree* tree, FILE* dumpFile)
{
    treeCtor(tree, dumpFile);

    treeElem_t elem = 
        {.value = '/', 
         .type  = DIF_NODE_TYPE_OPR};

    DifVar var =
        {.name = "x",
         .value = 1.0 };

    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = '/';
    tree->rootBranch->data = elem;

    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = '+';
    treeInsertRight(tree, tree->rootBranch, elem);

    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = '-';
    treeInsertLeft (tree, tree->rootBranch, elem);

    elem.type = DIF_NODE_TYPE_NUM;
    elem.value.num = 2000;
    treeInsertLeft (tree, tree->rootBranch->leftBranch, elem);
    elem.value.num = 1000;
    treeInsertRight(tree, tree->rootBranch->leftBranch, elem);

    elem.value.num = 3;
    treeInsertLeft (tree, tree->rootBranch->rightBranch, elem);
    elem.value.num = 4;
    treeInsertRight(tree, tree->rootBranch->rightBranch, elem);

}