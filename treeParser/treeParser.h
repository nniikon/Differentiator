#ifndef TREE_PARSER_H_
#define TREE_PARSER_H_

#include "../binaryTree/include/tree.h"

enum ParserError
{
    #define DEF_PARSER_ERR(err, msg) PARSER_ERR_ ## err,
    #include "treeParser_err_codegen.inc"
    #undef  DEF_PARSER_ERR
};

struct Token
{
    DifNode node;
    char bracket;
};

struct Parser
{
    MemDynArr variables;
    MemStack  tokens;

    FILE* logFile;
    ParserError error;
};

void parserPutTreeToFile(Tree* tree, FILE* file);
const char* parserGetErrMsg(ParserError err);
void parserPutNodeNameToFile(TreeNode* node, FILE* file);

ParserError parserLoadTreeFromFile(Parser* parser, Tree* tree, char* str);
ParserError parserCtor            (Parser* parser, FILE* logFile);
ParserError parserDtor            (Parser* parser);

#endif // TREE_PARSER_H_