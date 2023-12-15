#ifndef TREE_CFG_H_
#define TREE_CFG_H_

enum DifOprType
{
    #define DEF_DIF_OPR(opr, ...) DIF_OPR_ ## opr,
    #include "dif_operations_codegen.inc"
    #undef DEF_DIF_OPR

    DIF_OPR_END,
    DIF_OPR_INV = -1, // FIXME invalid
};

struct DifVar
{
    const char* name;
    double value;
    int length;
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
}; // FXIME put tot DifNode

// struct A
// {
//     union
//     {
//         int x;
//         double y;
//     };
// };

struct DifNode
{
    DifNodeValue value;
    DifNodeType  type;
    bool isConst;
};

typedef DifNode treeElem_t;

#endif // TREE_CFG_H_