#ifndef TREE_CFG_H_
#define TREE_CFG_H_

enum DifOprType
{
    #define DEF_DIF_OPR(opr, chr) DIF_OPR_ ## opr,
    #include "dif_operations_codegen.inc"
    #undef DEF_DIF_OPR
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
    bool isConst;
};

typedef DifNode treeElem_t;

#endif // TREE_CFG_H_