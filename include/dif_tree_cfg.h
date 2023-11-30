#ifndef TREE_CFG_H_
#define TREE_CFG_H_

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
    double  num;
    DifVar* var;
    char    opr;
};

struct DifNode
{
    DifNodeValue value;
    DifNodeType  type;
};

typedef DifNode treeElem_t;

#endif // TREE_CFG_H_