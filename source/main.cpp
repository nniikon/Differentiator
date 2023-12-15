#include <assert.h>
#include "../common/logs/logs.h"
#include "../binaryTree/include/tree.h"
#include "../include/dif.h"
#include "../include/dif_graphDump.h"
#include "../treeParser/treeParser.h"
#include "../common/fileToBuffer/fileToBuffer.h"
#include "../include/dif_latexLogs.h"

const char LOGS_FILE[]      = "logs.html";
const char DATA_FILE[]      = "db.dif";
const char LATEX_DIR[]      = "./tmp/tmp.tex";
const char LATEX_PDF_NAME[] = "./difLogs";
const char EQUATIONS_DIR[]  = "./tmp/dif_equations";

DifError Taylor(Dif* dif, Tree* tree, int nGraphs);

static FILE* openLogFile(const char* logFileName);

DifError differentiate(Dif* dif, Tree* tree);

int main()
{
    TreeError   errTree   = TREE_ERROR_NO;
    DifError    errCode   =    DIF_ERR_NO;
    ParserError errParser = PARSER_ERR_NO;

    FILE* logFile       = nullptr;
    FILE* texFile       = nullptr;
    FILE* databaseFile  = nullptr;

    Tree   tree   = {};
    Dif    dif    = {};
    Parser parser = {};

    size_t inputSize = 0ul;
    char* input = nullptr;

    logFile = openLogFile(LOGS_FILE);
    if (!logFile)
    {
        errCode = DIF_ERR_BAD_FOPEN;
        goto ReturnOpenLogFileFailure;
    }

    texFile = difLatexOpenFile(LATEX_DIR);
    if (!texFile)
    {
        errCode = DIF_ERR_BAD_FOPEN;
        goto ReturnOpenTexFileFailure;
    }

    databaseFile = fopen(DATA_FILE, "r");
    if (!databaseFile)
    {
        errCode = DIF_ERR_BAD_FOPEN;
        goto ReturnOpenDatabaseFileFailure;
    }

    errTree = treeCtor(&tree, logFile);
    if (errTree)
    {
        errCode = DIF_ERR_TREE;
        goto ReturnTreeCtorFailure;
    }

    errCode = difCtor(&dif, logFile, texFile);
    if (errCode)
    {
        goto ReturnDifCtorFailure;
    }

    errParser = parserCtor(&parser, logFile);
    if (errParser)
    {
        errCode = DIF_ERR_PARSER;
        goto ReturnParserCtorFailure;
    }

    input = putFileToBuffer(&inputSize, databaseFile);
    if (!input)
    {
        errCode = DIF_ERR_BAD_MEM_ALLOC;
        goto ReturnPutFileToBufferFailure;
    }

    // FIXME: add error checks
    errParser = parserLoadTreeFromFile(&parser, &tree, input);
    if (errParser)
    {
        errCode = DIF_ERR_PARSER;
        goto ReturnCleanUp;
    }

    //Taylor(&dif, &tree, 9);
    differentiate(&dif, &tree);

    ReturnCleanUp:

    free(input);
    ReturnPutFileToBufferFailure:
    parserDtor(&parser);
    ReturnParserCtorFailure:
    // difDtor(&dif);
    ReturnDifCtorFailure:
    treeDtor(&tree);
    ReturnTreeCtorFailure:
    fclose(databaseFile);
    ReturnOpenDatabaseFileFailure:
    difLatexCompileFile(texFile, LATEX_DIR, LATEX_PDF_NAME);
    ReturnOpenTexFileFailure:
    fclose(logFile);
    ReturnOpenLogFileFailure:

    return errCode;
}

static int factorial(int n)
{
    assert(n >= 0);

    if (n == 0)
        return 1;
    else
        return n * factorial(n - 1);
}


DifError differentiate(Dif* dif, Tree* tree)
{
    return difDifTreeDump(dif, tree);
}


DifError Taylor(Dif* dif, Tree* tree, int nGraphs)
{
    Tree* diffedTrees = (Tree*) calloc(nGraphs, sizeof(Tree));
    if (!diffedTrees)
        return DIF_ERR_BAD_MEM_ALLOC;

    double* diffedTreesValues = (double*) calloc(nGraphs, sizeof(double));
    if (!diffedTreesValues)
    {
        free(diffedTrees);
        return DIF_ERR_BAD_MEM_ALLOC;
    }

    FILE* equationsFile = fopen(EQUATIONS_DIR, "w");
    if (!equationsFile)
    {
        free(diffedTreesValues);
        free(diffedTrees);
        return DIF_ERR_BAD_FOPEN;
    }
    parserPutTreeToFile(tree, equationsFile);
    fputc('\n', equationsFile);

    DifError   difErr = DIF_ERR_NO; 
    TreeError treeErr = treeCpy(tree, &diffedTrees[0]);
    if (treeErr)
    {
        free(diffedTreesValues);
        fclose(equationsFile);
        free(diffedTrees);
        return DIF_ERR_TREE;
    }

    // Calculate all diffed trees
    for (int n = 1; n < nGraphs; n++)
    {
        treeErr = treeCpy(&diffedTrees[n - 1], &diffedTrees[n]);
        if (treeErr)
        {
            difErr = DIF_ERR_TREE;
            goto ReturnCleanUp;
        }

        diffedTrees[n].rootBranch = treeCreateNode(&diffedTrees[n], diffedTrees[n].rootBranch, nullptr, nullptr,
                                        {{.opr = DIF_OPR_DIF}, DIF_NODE_TYPE_OPR});
        if (!diffedTrees[n].rootBranch)
        {
            difErr = DIF_ERR_MEM_DYN_ARR;
            goto ReturnCleanUp;
        }

        difErr = difDifTreeDump(dif, &diffedTrees[n]);
        if (difErr)
        {
            goto ReturnCleanUp;
        }

        diffedTreesValues[n] = difEvalNode_recursive(&diffedTrees[n], diffedTrees[n].rootBranch, true, 0.0);
    }

    for (int n = 1; n < nGraphs; n++)
    {
        for (int k = 0; k <= n; k++)
        {
            fprintf(equationsFile, "%lg * x^%d / %d", diffedTreesValues[k], k, factorial(k));

            if (k != n)
                fprintf(equationsFile, " + ");
        }
        fputc('\n', equationsFile);
    }


    ReturnCleanUp:

    for (int i = 0; i < nGraphs; i++)
        treeDtor(&diffedTrees[i]);

    free(diffedTreesValues);
    free(diffedTrees);
    fclose(equationsFile);

    difLatexGenGraph();

    return difErr;
}


static FILE* openLogFile(const char* logFileName)
{
    FILE* logFile = fopen(logFileName, "w");
    if (logFile == nullptr)
        return nullptr;

    fprintf(logFile, "<pre style=\"background: #000000;color:#000000;\">");

    setvbuf(logFile, NULL, _IONBF, 0);

    return logFile;
}