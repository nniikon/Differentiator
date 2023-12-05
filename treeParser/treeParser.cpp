#include "treeParser.h"

#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include "tree_stack_cfg.h"
#include "../include/dif_operations.h"
#include "../common/logs/logs.h"
#include "../common/fileToBuffer/fileToBuffer.h"
#include "../common/stack/stack.h"
#include "../include/dif_hash.h"


const char* parserGetErrMsg(ParserError err)
{
    #define DEF_PARSER_ERR(err, msg)                                             \
        case PARSER_ERR_ ## err:                                                 \
            return msg;

    switch (err)
    {
        #include "treeParser_err_codegen.inc"
        default:
            return "No such error was found";
    }
    #undef DEF_PARSER_ERR
}


#define PARSER_LOG_RETURN_ERR(logFile, err)                                    \
    if (err)                                                                   \
        LOGF_ERR(logFile, "%s\n", parserGetErrMsg(err));                       \


///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// LOAD /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static bool isDelim(const char del)
{
    if (isdigit(del) || 
        isspace(del) || 
        del == '('   ||
        del == ')'      )
    {
        return true;
    }
    return false;
}


static int getWordSize(const char* input)
{
    int  i = 0;
    for (i = 0; input[i]; i++)
    {
        if (isDelim(input[i]))
            return i;
    }
    return i;
}


static const char* skipSpaces(const char* input)
{
    assert(input);

    size_t i = 0;
    for (i = 0; input[i]; i++)
    {
        if (!isspace(input[i]))
            return (input + i);
    }
    return input + i;
}


static DifOprType getOprType(const char* name, int size)
{
    long hash = difGetOprHashBySize(name, size);

    for (size_t type = 0; type < sizeof(DIF_OPERATIONS) / sizeof(DIF_OPERATIONS[0]); type++)
    {
        if (hash == DIF_OPERATIONS[type].hash)
            return (DifOprType) type;
    }
    return DIF_OPR_ERR;
}


static void printTokens(Stack* stk, FILE* logFile)
{
    LOG_START_COLOR(logFile, blue);
    fprintf(logFile, "\t Tokens dump: \n \t");
    for (int i = 0; i < stk->size; i++)
    {
        if (stk->data[i].bracket)
        {
            fprintf(logFile, "%c ", stk->data[i].bracket);
            continue;
        }
        switch (stk->data[i].node.type)
        {
            case DIF_NODE_TYPE_OPR:
                fprintf(logFile, "%s ", DIF_OPERATIONS[(int)stk->data[i].node.value.opr].name);
                break;
            case DIF_NODE_TYPE_NUM:
                fprintf(logFile, "%lg ", stk->data[i].node.value.num);
                break;
            case DIF_NODE_TYPE_VAR:
            default:
                assert(0); // Is yet to be added... 
                break;
        }
    }
    fputc('\n', logFile);
    LOG_END(logFile);
}


static bool tryOprToken(const char* str, Token* token, int* size, FILE* logFile)
{
    LOGF(logFile, "Trying to find an operation: %s\n", str);

    // K O C T bI JL b FIXME
    // First try for 1-symbol operations
    *size = 1;
    DifOprType oprType = getOprType(str, *size);
    if (oprType == DIF_OPR_ERR)
    {
        *size = getWordSize(str);
        LOGF(logFile, "Operation length: (%d)\n", *size); // FIXME strcmp 
        oprType = getOprType(str, *size);
    }

    token->node.type = DIF_NODE_TYPE_OPR;
    token->node.value.opr = getOprType(str, *size);

    if (token->node.value.opr != DIF_OPR_ERR)
    {
        LOGF_COLOR(logFile, cyan, "Detected operation: %s\n",
                DIF_OPERATIONS[(int)token->node.value.opr].name);
        return true;
    }

    return false;
}


static bool tryDigitToken(const char* str, Token* token, int* size, FILE* logFile)
{
    if (!isdigit(str[0]))
        return false;

    LOGF(logFile, "Trying to get a number: %s\n", str);

    char* numEnd = nullptr;
    double num = strtod(str, &numEnd);

    LOGF_COLOR(logFile, cyan, "Detected number: %lg\n", num);

    *size = (int)(numEnd - str);

    token->node.value.num = num;
    token->node.type      = DIF_NODE_TYPE_NUM;

    return true;
}


static bool tryBracketToken(const char* str, Token* token, int* size, FILE* logFile)
{
    if (str[0] != '(' && str[0] != ')')
        return false;

    LOGF_COLOR(logFile, cyan, "Detected bracket: %c\n", str[0]);
    *size = 1;
    token->bracket = str[0];
    return true;
}


static ParserError parseStrToTokens(Stack* stk, const char* str, FILE* logFile)
{
    LOG_FUNC_START(logFile);

    while (str[0])
    {
        LOGF(logFile, "Cycle started with: %s\n", str);

        Token newToken = {};

        str = skipSpaces(str);

        int tokenSize = 0;

        if (!tryBracketToken(str, &newToken, &tokenSize, logFile) &&
            !tryDigitToken  (str, &newToken, &tokenSize, logFile) && 
            !tryOprToken    (str, &newToken, &tokenSize, logFile)    )
        {
            LOGF_WRN(logFile, "Syntax error: %s\n", str);
        }


        str += tokenSize;
        stackPush(stk, newToken);
    }
    return PARSER_ERR_NO;
}




static TreeNode* getBracketsOprs(Tree* tree, const Stack* stk, size_t* pos, FILE* logFile);

// static TreeNode* getFirstOrderOprs(Tree* tree, const Stack* stk, size_t* pos, FILE* logFile)
// {
//     LOG_FUNC_START(logFile);

//     TreeNode* leftNode = getBracketsOprs(tree, stk, pos, logFile);

//     DifNode node = stk->data[*pos].node;

//     LOGF(logFile, "current oper order: %d\n", DIF_OPERATIONS[(int)node.value.opr].order);

//     while (!stk->data[*pos].bracket && DIF_OPERATIONS[(int)node.value.opr].order == 1) // ^
//     {
//         LOGF(logFile, "%s\n", "Found first order operation");
//         LOGF_COLOR(logFile, green, "The operations is %s\n", DIF_OPERATIONS[(int)node.value.opr].name);
//         (*pos)++;
//         TreeNode* rightNode = getBracketsOprs(tree, stk, pos, logFile);

//         leftNode = treeCreateNode(tree, leftNode, rightNode, nullptr, 
//                                  {{.opr = node.value.opr}, DIF_NODE_TYPE_OPR});
//     }

//     LOG_FUNC_END(logFile);
//     return leftNode;
// }


// static TreeNode* getThirdOrderOprs(Tree* tree, const Stack* stk, size_t* pos, FILE* logFile)
// {
//     LOG_FUNC_START(logFile);

//     TreeNode* leftNode = getFirstOrderOprs(tree, stk, pos, logFile);

//     DifNode node = stk->data[*pos].node;

//     LOGF(logFile, "current oper order: %d\n", DIF_OPERATIONS[(int)node.value.opr].order);

//     while (!stk->data[*pos].bracket && DIF_OPERATIONS[(int)node.value.opr].order == 3) // * or /
//     {
//         LOGF(logFile, "%s\n", "Found third order operation");
//         LOGF_COLOR(logFile, green, "The operations is %s\n", DIF_OPERATIONS[(int)node.value.opr].name);
//         (*pos)++;
//         TreeNode* rightNode = getFirstOrderOprs(tree, stk, pos, logFile);

//         leftNode = treeCreateNode(tree, leftNode, rightNode, nullptr, 
//                                  {{.opr = node.value.opr}, DIF_NODE_TYPE_OPR});
//     }

//     LOG_FUNC_END(logFile);
//     return leftNode;
// }


// static TreeNode* getForthOrderOprs(Tree* tree, const Stack* stk, size_t* pos, FILE* logFile)
// {
//     LOG_FUNC_START(logFile);

//     TreeNode* leftNode = getThirdOrderOprs(tree, stk, pos, logFile);

//     DifNode node = stk->data[*pos].node;

//     LOGF(logFile, "current oper order: %d\n", DIF_OPERATIONS[(int)node.value.opr].order);

//     while (!stk->data[*pos].bracket && DIF_OPERATIONS[(int)node.value.opr].order == 4) // + or -
//     {
//         LOGF(logFile, "%s\n", "Found forth order operations");
//         LOGF_COLOR(logFile, green, "The operations is %s\n", DIF_OPERATIONS[(int)node.value.opr].name);
//         (*pos)++;
//         TreeNode* rightNode = getThirdOrderOprs(tree, stk, pos, logFile);

//         leftNode = treeCreateNode(tree, leftNode, rightNode, nullptr, 
//                                  {{.opr = node.value.opr}, DIF_NODE_TYPE_OPR});
//     }

//     LOG_FUNC_END(logFile);
//     return leftNode;
// }


static TreeNode* getIndexOrderOprs(int order, Tree* tree, const Stack* stk, size_t* pos, FILE* logFile)
{
    if (*pos > stk->size)
        return nullptr;
    LOG_FUNC_START(logFile);

    TreeNode* leftNode = nullptr;
    if (order == 0)
        leftNode = getBracketsOprs(tree, stk, pos, logFile);
    else
        leftNode = getIndexOrderOprs(order - 1, tree, stk, pos, logFile);

    while (!stk->data[*pos].bracket && DIF_OPERATIONS[(int)stk->data[*pos].node.value.opr].order == order)
    {
        LOGF      (logFile,        "Found %d order operations at position %zu\n", order, *pos);
        LOGF_COLOR(logFile, green, "The operations is %s\n", DIF_OPERATIONS[(int)stk->data[*pos].node.value.opr].name);
        LOGF_COLOR(logFile, green, "Operation id: %d\n", (int)stk->data[*pos].node.value.opr);

        size_t oldPos = *pos;
        (*pos)++;

        TreeNode* rightNode = nullptr;
 
        if (order == 0)
            rightNode = getBracketsOprs(tree, stk, pos, logFile);
        else
            rightNode = getIndexOrderOprs(order - 1, tree, stk, pos, logFile);

        leftNode = treeCreateNode(tree, leftNode, rightNode, nullptr, 
                                 {{.opr = stk->data[oldPos].node.value.opr}, DIF_NODE_TYPE_OPR});
    }

    LOG_FUNC_END(logFile);
    return leftNode;
}


static TreeNode* getBracketsOprs(Tree* tree, const Stack* stk, size_t* pos, FILE* logFile)
{
    LOG_FUNC_START(logFile);

    TreeNode* node = nullptr;

    if (stk->data[*pos].bracket == '(')
    {
        (*pos)++;

        LOGF_COLOR(logFile, green, "%s\n", "Going inside the bracket!");
        node = getIndexOrderOprs(4, tree, stk, pos, logFile);
        LOGF_COLOR(logFile, green, "%s\n", "Out of the bracket!");
        if (stk->data[*pos].bracket != ')')
        {
            LOGF_WRN(logFile, "Syntax error at pos %zu\n", *pos);
            return nullptr;
        }

        (*pos)++;
    }
    else
    {
        node = treeCreateNode(tree, nullptr, nullptr, nullptr, stk->data[*pos].node);
        LOGF_COLOR(logFile, green,  "I got a number: %lg\n", stk->data[*pos].node.value.num);
        (*pos)++;
    }

    LOG_FUNC_END(logFile);
    return node;
}


ParserError parserLoadTreeFromFile(Tree* tree, FILE* file, FILE* logFile)
{
    LOG_FUNC_START(logFile);

    Stack tokens = {};
    ParserError prsErr = PARSER_ERR_NO;
    StackError  stkErr = STACK_NO_ERROR; // cringe
    size_t pos = 0;
    TreeNode* node = nullptr;

    size_t size = 0;
    char* input = putFileToBuffer(&size, file);
    if (!input)
    {
        prsErr = PARSER_ERR_FILE_TO_BUFFER;
        goto FileToBufferFailure;
    }
    LOGF(logFile, "INPUT: (%s)\n", input);


    stkErr = stackInit(&tokens);
    if (stkErr)
    {
        prsErr = PARSER_ERR_STACK;
        goto StackInitFailure;
    }

    prsErr = parseStrToTokens(&tokens, input, logFile);
    if (prsErr)
    {
        goto ParseStrToTokensFailure;
    }


    printTokens(&tokens, logFile);
    node = getIndexOrderOprs(4, tree, &tokens, &pos, logFile);
    printTokens(&tokens, logFile);
    tree->rootBranch = node;
    LOGF_COLOR(logFile, green, "Pos: %zu, size: %d\n", pos, tokens.size);

    LOG_FUNC_END(logFile);
    ParseStrToTokensFailure:
    stackDtor(&tokens); 
    StackInitFailure:
    free(input);
    FileToBufferFailure:
    PARSER_LOG_RETURN_ERR(logFile, prsErr);
    return prsErr;
}


///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// SAVE /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void putNodeToFile_recursive(Tree* tree, TreeNode* node, FILE* file);
static void putNodeNameToFile      (Tree* tree, TreeNode* node, FILE* file);

static void putNodeNameToFile(Tree* tree, TreeNode* node, FILE* file)
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


static void putNodeToFile_recursive(Tree* tree, TreeNode* node, FILE* file)
{

    if (!node)
        return;

    if (node->data.type == DIF_NODE_TYPE_OPR)
        fprintf(file, "(");

    putNodeToFile_recursive(tree, node->leftBranch , file);
    putNodeNameToFile      (tree, node             , file);
    putNodeToFile_recursive(tree, node->rightBranch, file);

    if (node->data.type == DIF_NODE_TYPE_OPR)
        fprintf(file, ")");
}


void parserPutTreeToFile(Tree* tree, FILE* file)
{
    LOG_FUNC_START(tree->debugInfo.dumpFile);
    assert(tree);
    if (!tree)
        return;
    
    putNodeToFile_recursive(tree, tree->rootBranch, file);

    LOG_FUNC_END(tree->debugInfo.dumpFile);
}