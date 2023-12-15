#include "../include/dif_latexLogs.h"

#include <assert.h>

#include "../treeParser/treeParser.h"
#include "../include/dif_operations.h"

const char LATEX_BEGINNING_PATH[] = "./tmp/dif_latexBeginning.tex";
const char    LATEX_ENDING_PATH[] = "./tmp/dif_latexEnding.tex";
const char LATEX_EQUATIONS_PATH[] = "./tmp/dif_equations";

FILE* difLatexOpenFile(const char* filePath)
{
    assert(filePath);

    FILE* file = fopen(filePath, "w");
    if (!file)
        return nullptr;

    fprintf(file, "\\input %s\n", LATEX_BEGINNING_PATH);

    return file;
}


void difLatexCompileFile(FILE* file, const char* filePath,
                                     const char* outputPath)
{
    assert(file);

    fprintf(file, "\\input %s\n", LATEX_ENDING_PATH);

    fclose(file);

    const int cmdBufferSize = 100;
    char cmdBuffer[cmdBufferSize] = {};

    snprintf(cmdBuffer, cmdBufferSize, "pdflatex -jobname=%s -o %s", outputPath, filePath);
    system(cmdBuffer);
}


static void difLatexDumpNode_recursive(Dif* dif, Tree* tree, TreeNode* node, int prevOprOrder)
{
    if (!node || !dif)
        return;

    if (node->data.type != DIF_NODE_TYPE_OPR)
    {
        difLatexDumpNode_recursive(dif, tree, node-> leftBranch, prevOprOrder);
        parserPutNodeNameToFile (node, dif->outputFile);
        difLatexDumpNode_recursive(dif, tree, node->rightBranch, prevOprOrder);
        return;
    }
    int curOprOrder = getDifOpr(node)->order;

    // Cringe, but since DIF is the only operation of this type,
    // I think it's a good K O C T bI JL b. 
    if (node->data.value.opr == DIF_OPR_DIF)
    {
        fprintf(dif->outputFile, "(");

        difLatexDumpNode_recursive(dif, tree, node->leftBranch, curOprOrder);

        fprintf(dif->outputFile, ")");
        fprintf(dif->outputFile, "\'"); // cringe but that's the way it is.
        return;
    }
    // FIXME FIX COPYPASTE
    if (getDifOpr(node)->isFunctionLike)
    {
        parserPutNodeNameToFile(node, dif->outputFile);
        fprintf(dif->outputFile, "(");
        difLatexDumpNode_recursive(dif, tree, node->leftBranch , curOprOrder);
        fprintf(dif->outputFile, ")");
        return;
    }

    switch (node->data.value.opr)
    {
        case DIF_OPR_MUL:
            if (curOprOrder > prevOprOrder)
                fprintf(dif->outputFile, "(");

            difLatexDumpNode_recursive(dif, tree, node->leftBranch, curOprOrder);
            fprintf(dif->outputFile, "\\cdot ");
            difLatexDumpNode_recursive(dif, tree, node->rightBranch, curOprOrder);

            if (curOprOrder > prevOprOrder)
                fprintf(dif->outputFile, ")");

            break;

        case DIF_OPR_DIV: 
            fprintf(dif->outputFile, "\\frac{");
            difLatexDumpNode_recursive(dif, tree, node->leftBranch,  __INT_MAX__);
            fprintf(dif->outputFile, "}{");
            difLatexDumpNode_recursive(dif, tree, node->rightBranch, __INT_MAX__);
            fprintf(dif->outputFile, "} ");
            break;

        case DIF_OPR_POW:
            fprintf(dif->outputFile, "{(");
            difLatexDumpNode_recursive(dif, tree, node->leftBranch ,  __INT_MAX__);
            fprintf(dif->outputFile, ")}^{");
            difLatexDumpNode_recursive(dif, tree, node->rightBranch,  __INT_MAX__);
            fprintf(dif->outputFile, "} ");
            break;

        // WARNING FIXME

        default:
            if (curOprOrder > prevOprOrder)
                fprintf(dif->outputFile, "(");

            difLatexDumpNode_recursive(dif, tree, node->leftBranch, curOprOrder);
            parserPutNodeNameToFile(node, dif->outputFile);
            difLatexDumpNode_recursive(dif, tree, node->rightBranch, curOprOrder);

            if (curOprOrder > prevOprOrder)
                fprintf(dif->outputFile, ")");

            break;
    }
}


void difLatexDumpTree(Dif* dif, Tree* tree)
{
    if (!dif || !tree)
        return;
    
    fprintf(dif->outputFile, "$$ ");
    difLatexDumpNode_recursive(dif, tree, tree->rootBranch, __INT_MAX__);
    fprintf(dif->outputFile, "$$\n\n");
    fputc('\n', dif->outputFile);
}


void difLatexGenGraph()
{
    system("python3 ./tmp/dif_graphenizer.py");
}