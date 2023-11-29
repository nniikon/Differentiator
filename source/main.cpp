#include "../common/logs/logs.h"
#include "../binaryTree/include/tree.h"

int main()
{
    FILE* file = fopen("logs.html", "w");
    if (file == nullptr)
        return -1;
    fprintf(file, "<pre>");

    Tree tree = {};
    treeCtor(&tree, file);
    treeInsertRight(&tree, tree.rootBranch,  10);

    treeInsertLeft (&tree, tree.rootBranch, 15);

    treeInsertRight(&tree, tree.rootBranch->leftBranch,  20);

    treeInsertLeft (&tree, tree.rootBranch->leftBranch, 25);

    Tree dst = {};
    treeCpy(&tree, &dst);
}