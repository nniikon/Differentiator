#include "treeParser.h"

#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "../include/dif_operations.h"
#include "../common/logs/logs.h"
#include "../common/fileToBuffer/fileToBuffer.h"


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
    if ((err) != PARSER_ERR_NO)                                                \
        LOGF_ERR(logFile, "%s\n", parserGetErrMsg(err));                       \


ParserError parserCtor(Parser* parser, FILE* logFile)
{
    if (!logFile || !parser)
        PARSER_LOG_RETURN_ERR(logFile, PARSER_ERR_NULLPTR_PASSED);

    LOG_FUNC_START(logFile);

    parser->logFile = logFile;

    if (memDynArrCtor(&parser->variables, sizeof(DifVar)))
    {
        PARSER_LOG_RETURN_ERR(logFile, PARSER_ERR_MEM_DYN_ARR);
    }

    if (memStackCtor(&parser->tokens, sizeof(Token)))
    {
        memDynArrDtor(&parser->variables);
        PARSER_LOG_RETURN_ERR(logFile, PARSER_ERR_MEM_STACK);
    }

    LOG_FUNC_END(logFile);
    return PARSER_ERR_NO;
}


ParserError parserDtor(Parser* parser)
{
    if (!parser)
        PARSER_LOG_RETURN_ERR(nullptr, PARSER_ERR_NULLPTR_PASSED);
    
    LOG_FUNC_START(parser->logFile);

    memDynArrDtor(&parser->variables);
    memStackDtor (&parser->tokens);

    LOG_FUNC_END(parser->logFile);
    return PARSER_ERR_NO;
}


///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// LOAD /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool isVarDelim(const char del)
{
    if (isalnum(del))
        return false;

    return true;
}


static int getVarSize(const char* str)
{
    assert(str);

    int i = 0;
    for (; str[i]; i++)
    {
        if (isVarDelim(str[i]))
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


static DifOprType getOprType(const char* name, int* size)
{
    for (size_t type = 0; type < DIF_N_OPERATIONS; type++)
    {
        if (strncmp(name, DIF_OPERATIONS[type].name, DIF_OPERATIONS[type].size) == 0)
        {
            *size = DIF_OPERATIONS[type].size;
            return (DifOprType) type;
        }
    }
    return DIF_OPR_INV;
}


static void printTokens(MemStack* tokenStk, FILE* logFile)
{
    LOG_START_COLOR(logFile, blue);

    Token* tokens = (Token*) tokenStk->data;

    fprintf(logFile, "\t Tokens dump: \n \t");
    fprintf(logFile, "| ");
    for (unsigned int i = 0; i < tokenStk->size; i++)
    {
        if (tokens[i].bracket)
        {
            fprintf(logFile, "%c | ", tokens[i].bracket);
            continue;
        }

        switch (tokens[i].node.type)
        {
            case DIF_NODE_TYPE_OPR:
                fprintf(logFile, "%s | ", DIF_OPERATIONS[(int)tokens[i].node.value.opr].name);
                break;
            case DIF_NODE_TYPE_NUM:
                fprintf(logFile, "%lg | ", tokens[i].node.value.num);
                break;
            case DIF_NODE_TYPE_VAR:
                fprintf(logFile, "%.*s | ", tokens[i].node.value.var->length,
                                            tokens[i].node.value.var->name);
                break;
            default:
                assert(0);
                break;
        }
    }
    fputc('\n', logFile);

    LOG_END(logFile);
}


static bool tryOprToken(const char* str, Token* token, int* size, FILE* logFile)
{
    LOGF(logFile, "Trying to find an operation: %s\n", str);

    token->node.type = DIF_NODE_TYPE_OPR;
    token->node.value.opr = getOprType(str, size);

    if (token->node.value.opr != DIF_OPR_INV)
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


static bool tryBracketToken(const char* str, Token* tokens, int* size, FILE* logFile)
{
    if (str[0] != '(' && str[0] != ')')
        return false;

    LOGF_COLOR(logFile, cyan, "Detected bracket: %c\n", str[0]);
    *size = 1;
    tokens->bracket = str[0];
    return true;
}


static bool tryVarToken(const char* str, Token* tokens, int* size, FILE* logFile, MemDynArr* vars)
{
    assert(vars);
    assert(str);
    LOGF(logFile, "Trying to get a variable: %s\n", str);

    DifVar* var = nullptr;
    int varLength = getVarSize(str);
    LOGF_COLOR(logFile, cyan, "Detected varible: %.*s\n", varLength, str);
    LOGF_COLOR(logFile, cyan, "Variable size: %d\n", varLength);

    bool isVarFound = false;
    while((var = (DifVar*) memDynArrGetElem(vars)) != nullptr)
    {
        if (var->length != varLength)
            continue;

        if (strncmp(var->name, str, varLength) == 0)
        {
            isVarFound = true;
            break;
        }
    }
    memDynArrGetElemReset(vars);

    // If the variable hasn't been found before, create it.
    if (!isVarFound)
    {
        var = (DifVar*) memDynArrCalloc(vars);
        if (!var)
        {
            LOGF_ERR(logFile, "%s\n", parserGetErrMsg(PARSER_ERR_MEM_DYN_ARR));
            // FIXME: error handling
        }
        LOGF_COLOR(logFile, cyan, "%s\n", "Created a new varible");
        var->name = str;
        var->length = varLength;
    }

    tokens->node.type = DIF_NODE_TYPE_VAR;
    tokens->node.value.var = var;

    *size = varLength;

    return true;
}


static ParserError parseStrToTokens(MemStack* tokens, MemDynArr* vars, const char* str, FILE* logFile)
{
    LOG_FUNC_START(logFile);

    while (str[0])
    {
        LOGF(logFile, "Cycle started with: %s\n", str);

        Token newToken = {};

        str = skipSpaces(str);

        int tokenSize = 0;

        if (!tryBracketToken(str, &newToken, &tokenSize, logFile      ) &&
            !tryDigitToken  (str, &newToken, &tokenSize, logFile      ) && 
            !tryOprToken    (str, &newToken, &tokenSize, logFile      ) &&
            !tryVarToken    (str, &newToken, &tokenSize, logFile, vars)    )
        {
            LOGF_WRN(logFile, "Syntax error: %s\n", str);
        }

        str += tokenSize;
        memStackPush(tokens, &newToken);
    }

    Token endToken = {};

    endToken.node.type = DIF_NODE_TYPE_OPR;
    endToken.node.value.opr = DIF_OPR_END;
    memStackPush(tokens, &endToken);

    return PARSER_ERR_NO;
}


static TreeNode* getFunctionLikeOprs(Tree* tree, const Token* tokens, size_t* pos, FILE* logFile);

static TreeNode* getBracketsOprs(Tree* tree, const Token* tokens, size_t* pos, FILE* logFile);

static TreeNode* getIndexOrderOprs(int order, Tree* tree, const Token* tokens, size_t* pos, FILE* logFile)
{
    LOG_FUNC_START(logFile);

    TreeNode* leftNode = nullptr;
    if (order == 0)
        leftNode = getBracketsOprs(tree, tokens, pos, logFile);
    else
        leftNode = getIndexOrderOprs(order - 1, tree, tokens, pos, logFile);

    while (!tokens[*pos].bracket && DIF_OPERATIONS[(int)tokens[*pos].node.value.opr].order == order) // FIXME
    {
        LOGF      (logFile,        "Found %d order operations at position %zu\n", order, *pos);
        LOGF_COLOR(logFile, green, "The operations is %s\n", DIF_OPERATIONS[(int)tokens[*pos].node.value.opr].name);
        LOGF_COLOR(logFile, green, "Operation id: %d\n", (int)tokens[*pos].node.value.opr);

        size_t oldPos = *pos;
        (*pos)++;

        TreeNode* rightNode = nullptr;
        if (DIF_OPERATIONS[(int)tokens[oldPos].node.value.opr].isBinary)
        {
            if (order == 0)
                rightNode = getBracketsOprs(tree, tokens, pos, logFile);
            else
                rightNode = getIndexOrderOprs(order - 1, tree, tokens, pos, logFile);
        }
        else
        {
            assert(0); // is yet to be implemented...
        }

        leftNode = treeCreateNode(tree, leftNode, rightNode, nullptr, 
                                 {{.opr = tokens[oldPos].node.value.opr}, DIF_NODE_TYPE_OPR});
    }

    LOG_FUNC_END(logFile);
    return leftNode;
}


static TreeNode* getFunctionLikeOprs(Tree* tree, const Token* tokens, size_t* pos, FILE* logFile)
{
    size_t oldPos = *pos;
    (*pos)++;
    if (tokens[*pos].bracket != '(')
    {
        LOGF_WRN(logFile, "%s\n", "syntax error"); // FIXME
    }
    (*pos)++;

    TreeNode* leftNode = getIndexOrderOprs(4, tree, tokens, pos, logFile);
    TreeNode* node = treeCreateNode(tree, leftNode, nullptr, nullptr, 
                                {{.opr = tokens[oldPos].node.value.opr}, DIF_NODE_TYPE_OPR});

    if (tokens[*pos].bracket != ')')
    {
        LOGF_WRN(logFile, "%s\n", "syntax error"); // FIXME
    }
    (*pos)++;

    return node;
}


static TreeNode* getBracketsOprs(Tree* tree, const Token* tokens, size_t* pos, FILE* logFile)
{
    LOG_FUNC_START(logFile);

    TreeNode* node = nullptr;

    if (tokens[*pos].bracket == '(')
    {
        (*pos)++;

        LOGF_COLOR(logFile, green, "%s\n", "Going inside the bracket!");
        node = getIndexOrderOprs(4, tree, tokens, pos, logFile); // 4 eto mem ya uberu
        LOGF_COLOR(logFile, green, "%s\n", "Out of the bracket!");
        if (tokens[*pos].bracket != ')')
        {
            LOGF_WRN(logFile, "Syntax error at pos %zu\n", *pos);
            return nullptr;
        }

        (*pos)++;
    }
    else if (tokens[*pos].node.type == DIF_NODE_TYPE_OPR &&
             DIF_OPERATIONS[(int)tokens[*pos].node.value.opr].isFunctionLike)
    {
        node = getFunctionLikeOprs(tree, tokens, pos, logFile);
    }
    else if (tokens[*pos].node.type == DIF_NODE_TYPE_VAR)
    {
        node = treeCreateNode(tree, nullptr, nullptr, nullptr, tokens[*pos].node);
        LOGF_COLOR(logFile, green, "I got a varibale: %.*s\n", tokens[*pos].node.value.var->length,
                                                               tokens[*pos].node.value.var->name);
        (*pos)++;
    }
    else
    {
        node = treeCreateNode(tree, nullptr, nullptr, nullptr, tokens[*pos].node);
        LOGF_COLOR(logFile, green,  "I got a number: %lg\n",   tokens[*pos].node.value.num);
        (*pos)++;
    }

    LOG_FUNC_END(logFile);
    return node;
}


static ParserError parseTokenToTree(Tree* tree, Token* tokens, FILE* logFile)
{
    size_t pos = 0;
    tree->rootBranch = getIndexOrderOprs(4, tree, tokens, &pos, logFile);
    if (tokens[pos].node.type != DIF_NODE_TYPE_OPR || 
        tokens[pos].node.value.opr != DIF_OPR_END)
    {
        LOGF_WRN(logFile, "%s\n", "Syntax error"); // FIXME: add syntax errors
    }
    LOGF_COLOR(logFile, green, "Parsing tokens success: %zu\n", pos);

    return PARSER_ERR_NO;
}


ParserError parserLoadTreeFromFile(Parser* parser, Tree* tree, char* str)
{
    LOG_FUNC_START(parser->logFile);

    ParserError prsErr = parseStrToTokens(&parser->tokens, &parser->variables,
                                                       str, parser->logFile);
    if (prsErr)
        PARSER_LOG_RETURN_ERR(parser->logFile, prsErr);

    printTokens(&parser->tokens, parser->logFile);

    parseTokenToTree(tree, (Token*) parser->tokens.data, parser->logFile);

    LOG_FUNC_END(parser->logFile);

    return prsErr;
}


///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// SAVE /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static void parserPutNodeToFile_recursive(TreeNode* node, FILE* file, int prevOprOrder);


void parserPutNodeNameToFile(TreeNode* node, FILE* file)
{
    assert(node);
    assert(file);

    switch(node->data.type)
    {
        case DIF_NODE_TYPE_NUM:
            fprintf(file, "%lg", node->data.value.num);
            break;
        case DIF_NODE_TYPE_OPR:
            fprintf(file, "%s", getDifOpr(node)->name);
            break;
        case DIF_NODE_TYPE_VAR:
            fprintf(file, "%.*s", node->data.value.var->length, 
                                  node->data.value.var->name   ); 
            break;
        default:
            assert(0);
            return;
    }
}


static void parserPutNodeToFile_recursive(TreeNode* node, FILE* file, int prevOprOrder)
{
    assert(file);

    if (!node)
        return;

    if (node->data.type != DIF_NODE_TYPE_OPR)
    {
        parserPutNodeToFile_recursive(node-> leftBranch, file, prevOprOrder);
        parserPutNodeNameToFile      (node,              file              );
        parserPutNodeToFile_recursive(node->rightBranch, file, prevOprOrder);
        return;
    }
    int curOprOrder = getDifOpr(node)->order;

    if (node->data.type == DIF_NODE_TYPE_OPR && getDifOpr(node)->isFunctionLike)
    {
        parserPutNodeNameToFile(node, file);
        fprintf(file, "(");
        parserPutNodeToFile_recursive(node-> leftBranch, file, curOprOrder);
        fprintf(file, ")");
        return;
    }

    if (node->data.type == DIF_NODE_TYPE_OPR && curOprOrder > prevOprOrder)
        fprintf(file, "(");

    parserPutNodeToFile_recursive(node-> leftBranch, file, curOprOrder);
    parserPutNodeNameToFile      (node,              file             );
    parserPutNodeToFile_recursive(node->rightBranch, file, curOprOrder);

    if (node->data.type == DIF_NODE_TYPE_OPR && curOprOrder > prevOprOrder)
        fprintf(file, ")");
}


void parserPutTreeToFile(Tree* tree, FILE* file)
{
    assert(tree);
    if (!tree || !file)
        return;
    parserPutNodeToFile_recursive(tree->rootBranch, file, __INT_MAX__);
}