#ifndef MEM_ALLOCATIONS_H_
#define MEM_ALLOCATIONS_H_

#include <stdlib.h>


typedef struct
{
    void** buffer;
    unsigned int row;
    unsigned int freeIndex;
    size_t elemSize;
} DynArr;


// /**
//  * @brief Dynamically allocates memory for an array and initializes it to zero.
//  *
//  * This function checks if the array needs to be resized based on the freeIndex and capacity.
//  * If necessary, it reallocates memory with a larger capacity, initializes the new elements to zero,
//  * and updates the capacity. It then returns a pointer to the element at the freeIndex,
//  * updating the freeIndex value.
//  * If the given array is NULL, the memory for it will be allocated.
//  *
//  * @param arr        Pointer to the array.
//  * @param capacity   Pointer to the size_t variable holding the current capacity of the array.
//  * @param freeIndex  The index of the next free element in the array.
//  * @param elemSize   The size of each element in the array.
//  * @return           void* Pointer to the element at the freeIndex.
//  * 
//  * @note You can configurate the behaviour of this function in the `memAllocations_cfg.h' file.
//  */

void* dynArrCalloc(DynArr* arr);
int   dynArrCtor  (DynArr* arr, size_t elemSize);
int   dynArrDtor  (DynArr* arr);

#endif // MEM_ALLOCATIONS_H_