#include "memAllocations.h"

#include <string.h>
#include <stdio.h>
#include "memAllocations_cfg.h"

void* dynArrCalloc(DynArr* arr)
{
    if (arr->freeIndex >  DYN_ARR_CAPACITY     ||
        arr->row       >= DYN_ARR_BUFFER_CAPACITY)
    {
        return NULL;
    }

    if (arr->freeIndex == DYN_ARR_CAPACITY)
    {
        arr->row++;
        if (arr->row == DYN_ARR_BUFFER_CAPACITY)
            return NULL;
        arr->freeIndex = 0;

        arr->buffer[arr->row] = (void*) calloc(DYN_ARR_CAPACITY, arr->elemSize);
        if (arr->buffer[arr->row] == NULL)
            return NULL;
    }

    return (void*)((size_t)arr->buffer[arr->row] + arr->elemSize * (arr->freeIndex)++);
}


int dynArrCtor(DynArr* arr, size_t elemSize)
{
    if (arr == NULL)
        return -1;

    arr->buffer = (void**) calloc(DYN_ARR_BUFFER_CAPACITY, sizeof(void*));
    if (arr->buffer == NULL)
        return -1;

    arr->buffer[0] = (void*) calloc(DYN_ARR_CAPACITY, elemSize);
    if (arr->buffer[0] == NULL)
        return -1;

    arr->elemSize  = elemSize;
    arr->freeIndex = 0;
    arr->row       = 0;

    return 0;
}


int dynArrDtor(DynArr* arr)
{
    if (arr == NULL)
        return -1;

    if (arr->buffer == NULL)
        return -1;

    for (unsigned int i = 0; i <= arr->row; i++)
        free(arr->buffer[i]);

    free(arr->buffer);

    return 0;
}