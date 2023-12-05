#ifndef TREE_PARSER_H_
#define TREE_PARSER_H_

#include "../binaryTree/include/tree.h"

void parserPutTreeToFile(Tree* tree, FILE* file);

enum ParserError
{
    #define DEF_PARSER_ERR(err, msg) PARSER_ERR_ ## err,
    #include "treeParser_err_codegen.inc"
    #undef  DEF_PARSER_ERR
};

const char* parserGetErrMsg(ParserError err);

ParserError parserLoadTreeFromFile(Tree* tree, FILE* file, FILE* logFile);

#endif // TREE_PARSER_H_