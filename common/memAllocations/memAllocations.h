#ifndef MEM_ALLOCATIONS_H_
#define MEM_ALLOCATIONS_H_

#include <stddef.h>

typedef struct
{
    void** buffer;
    unsigned int row;
    unsigned int freeIndex;
    size_t elemSize;
    unsigned int curRow;
    unsigned int curFreeIndex;
} MemDynArr;

typedef struct
{
    size_t elemSize;
    void* data;
    unsigned int size;
    unsigned int capacity;
} MemStack;

int   memDynArrCtor        (MemDynArr* arr, size_t elemSize);
int   memDynArrDtor        (MemDynArr* arr);
void* memDynArrCalloc      (MemDynArr* arr);
void* memDynArrGetElem     (MemDynArr* arr);
void  memDynArrGetElemReset(MemDynArr* arr);

int   memStackCtor(MemStack* stk, size_t elemSize); 
int   memStackDtor(MemStack* stk);
void* memStackPop (MemStack* str);
int   memStackPush(MemStack* str, void* elem);

#endif // MEM_ALLOCATIONS_H_