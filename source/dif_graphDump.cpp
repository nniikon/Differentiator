#include "../include/dif_graphDump.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>

#include "../include/dif.h"
#include "../include/dif_cfg.h"
#include "../include/dif_tree_cfg.h"

const int INT_MAX_LENGTH = 10;

static void difMakeLogdir()
{
    const char cmdMask[] = "mkdir %s -p";
    constexpr size_t cmdBufferSize = sizeof(cmdMask) + sizeof(DIF_DUMP_DIR) + 1;

    char cmdBuffer[cmdBufferSize] = {};
    snprintf(cmdBuffer, cmdBufferSize, cmdMask, DIF_DUMP_DIR);

    system(cmdBuffer);
}


static void difCompileDot(Dif* dif)
{
    constexpr size_t cmdBufferSize = 
        2 * INT_MAX_LENGTH + sizeof(DIF_DOT_FILE_MASK) + sizeof(DIF_PNG_FILE_MASK) + 16;

    char cmdBuffer[cmdBufferSize] = {};
    snprintf(cmdBuffer, cmdBufferSize,
                    "dot " DIF_DOT_FILE_MASK " -Tpng -o " DIF_PNG_FILE_MASK,
                    dif->dumpIndex, dif->dumpIndex);
    system(cmdBuffer);
}


#define log(...) fprintf(dotFile, __VA_ARGS__)

static void akinatorPrintNodes(Dif* dif, TreeNode* node, FILE* dotFile)
{
    LOG_FUNC_START(dif->logFile);
    const char* color = nullptr;
    const char* valueMask  = nullptr;

    const int valueBufferSize = 15;
    char valueStr[valueBufferSize] = {};
    switch (node->data.type)
    {
        case DIF_NODE_TYPE_NUM:
            color = DIF_NUM_COLOR;
            snprintf(valueStr, valueBufferSize, "%lg", node->data.value.num);
            break;

        case DIF_NODE_TYPE_OPR:
            color = DIF_OPR_COLOR;
            snprintf(valueStr, valueBufferSize, "%c", node->data.value.opr);
            break;

        case DIF_NODE_TYPE_VAR:
            color = DIF_VAR_COLOR;
            snprintf(valueStr, valueBufferSize, "%s", node->data.value.var->name);
            break;
        default: assert(0);
    };
    // QUESTION: FIXME: cringe?
    const char* const valueMaskLiteral = valueMask;

    log("\"%p\" [shape = Mrecord, style = filled, fillcolor = \"%s\", color = \"%s\", ", 
                    node, color, DIF_SCD_COLOR);
    log("label = \"{adress: %p | DATA: ", node);

    log("%s", valueStr);

    log(" | {<leftPrt> LEFT: %p | <rightPrt> RIGHT: %p}}\"];\n",
                                        node->leftBranch, node->rightBranch);
    if (node->leftBranch != nullptr)
    {
        akinatorPrintNodes(dif, node->leftBranch, dotFile);
        log("\"%p\":<leftPrt> -> \"%p\"[color = \"%s\", weight = 1]\n",
                                        node, node->leftBranch, DIF_SCD_COLOR);
    }
    if (node->rightBranch != nullptr)
    {
        akinatorPrintNodes(dif, node->rightBranch, dotFile);
        log("\"%p\":<rightPrt> -> \"%p\"[color = \"%s\", weight = 1]\n",
                                        node, node->rightBranch, DIF_SCD_COLOR);
    }
    LOG_FUNC_END(dif->logFile);
    return;
}


DifError difGraphDump(Dif* dif, Tree* tree)
{
    LOG_FUNC_START(dif->logFile);

    if (dif == nullptr)
        DIF_RETURN_LOG_ERROR(DIF_ERR_NULLPTR_PASSED, dif);

    if (dif->logFile == nullptr)
        DIF_RETURN_LOG(DIF_ERR_NO, dif);

    difMakeLogdir();

    constexpr int bufferSize = sizeof(DIF_DOT_FILE_MASK) + INT_MAX_LENGTH + 1;

    char fileBuffer[bufferSize] = {};
    snprintf(fileBuffer, bufferSize, DIF_DOT_FILE_MASK, dif->dumpIndex);

    LOGF(dif->logFile, "trying to open %s\n", fileBuffer);

    FILE* dotFile = fopen(fileBuffer, "w");
    if (dotFile == nullptr)
    {
        fclose(dotFile);
        DIF_RETURN_LOG_ERROR(DIF_ERR_BAD_FOPEN, dif);
    }
    log("digraph G{\n"
        "rankdir = TB;\n"
        "bgcolor = \"%s\";\n", DIF_BGR_COLOR);

    akinatorPrintNodes(dif, tree->rootBranch, dotFile);
    log("}\n");

    if (fclose(dotFile) != 0)
        LOGF_WRN(dif->logFile, "error closing %s\n", fileBuffer);
    
    difCompileDot(dif);

    dif->dumpIndex++;


    LOG_FUNC_END(dif->logFile);
    return DIF_ERR_NO;
}