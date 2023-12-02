#include "../common/logs/logs.h"
#include "../binaryTree/include/tree.h"
#include "../include/dif.h"
#include "../include/dif_graphDump.h"
#include "../treeParser/treeParser.h"

void createTestTree1(Tree* tree, FILE* dumpFile);
void createTestTree2(Tree* tree, FILE* dumpFile);
void createTestTree3(Tree* tree, FILE* dumpFile);

int main()
{
    FILE* logFile = fopen("logs.html", "w");
    if (logFile == nullptr)
        return -1;
    fprintf(logFile, "<pre style=\"background: #000000;color:#000000;\">");
    setvbuf(logFile, NULL, _IONBF, 0);

    Dif dif = {};
    difCtor(&dif, logFile);

    Tree tree = {};
    createTestTree2(&tree, logFile);

    difGraphDump(&dif, &tree);

    difDifTree        (&tree);

    treeSetParents    (&tree);
    difSimplify       (&tree);
    difGraphDump(&dif, &tree);

    FILE* databaseFile = fopen("db.dif", "w");
    if (databaseFile == nullptr)
    {
        fclose(logFile);
        return -1;
    }

    parserPutTreeToFile(&tree, databaseFile);
    

    treeDtor(&tree);

    fclose(logFile);
}

DifVar x =
    {.name = "x",
     .value = 1.0 };

void createTestTree1(Tree* tree, FILE* dumpFile)
{
    treeCtor(tree, dumpFile);

    treeElem_t elem = {};

    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = DIF_OPR_POW;
    tree->rootBranch->data = elem;

    elem.type = DIF_NODE_TYPE_VAR;
    elem.value.var = &x;
    treeInsertLeft (tree, tree->rootBranch, elem);

    elem.type = DIF_NODE_TYPE_NUM;
    elem.value.num = 2;
    treeInsertRight(tree, tree->rootBranch, elem);

    // elem.type = DIF_NODE_TYPE_NUM;
    // elem.value.num = 2000;
    // treeInsertLeft (tree, tree->rootBranch->leftBranch, elem);
    // elem.value.num = 1000;
    // treeInsertRight(tree, tree->rootBranch->leftBranch, elem);

    // elem.value.num = 3;
    // treeInsertLeft (tree, tree->rootBranch->rightBranch, elem);
    // elem.type = DIF_NODE_TYPE_VAR;
    // elem.value.var = &var;
    // treeInsertRight(tree, tree->rootBranch->rightBranch, elem);

}


void createTestTree2(Tree* tree, FILE* dumpFile)
{
    treeCtor(tree, dumpFile);

    treeElem_t elem = {};

    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = DIF_OPR_POW;
    tree->rootBranch->data = elem;

    elem.type = DIF_NODE_TYPE_VAR;
    elem.value.var = &x;
    treeInsertLeft (tree, tree->rootBranch, elem);

    elem.type = DIF_NODE_TYPE_NUM;
    elem.value.num = 1;
    treeInsertRight(tree, tree->rootBranch, elem);
}

void createTestTree3(Tree* tree, FILE* dumpFile)
{
    treeCtor(tree, dumpFile);

    treeElem_t elem = {};

    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = DIF_OPR_MUL;
    tree->rootBranch->data = elem;

    elem.type = DIF_NODE_TYPE_OPR;
    elem.value.opr = DIF_OPR_SUB;
    tree->rootBranch->data = elem;
    treeInsertLeft (tree, tree->rootBranch, elem);

    elem.type = DIF_NODE_TYPE_VAR;
    elem.value.var = &x;
    treeInsertRight(tree, tree->rootBranch, elem);

    elem.type = DIF_NODE_TYPE_NUM;
    elem.value.num = 1;
    treeInsertLeft (tree, tree->rootBranch->leftBranch, elem);

    elem.type = DIF_NODE_TYPE_NUM;
    elem.value.num = 5;
    treeInsertRight(tree, tree->rootBranch->leftBranch, elem);
}