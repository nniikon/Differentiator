#ifndef TREE_CFG_H_
#define TREE_CFG_H_

enum DifOprType
{
    DIF_OPR_ADD = 0,
    DIF_OPR_SUB = 1,
    DIF_OPR_MUL = 2,
    DIF_OPR_DIV = 3,
};

struct DifVar
{
    const char* name;
    double      value;
};

enum DifNodeType
{
    DIF_NODE_TYPE_NUM,
    DIF_NODE_TYPE_OPR,
    DIF_NODE_TYPE_VAR,
};

union DifNodeValue
{
    double     num;
    DifVar*    var;
    DifOprType opr;
};

struct DifNode
{
    DifNodeValue value;
    DifNodeType  type;
};

typedef DifNode treeElem_t;

#endif // TREE_CFG_H_