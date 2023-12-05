#ifndef TREE_STACK_CFG_H_
#define TREE_STACK_CFG_H_

#include "treeParser.h"

struct Token
{
    DifNode node;
    char bracket;
};

/** Release Mode
 * - Optimizes the code for performance.
 * - Does not fill unused hash values with poison.
 * - Validates only essential parameters.
 */
#define RELEASE

/** Canary Protection
 * - Enhances security using canary values to protect the stack and data.
 * - Slightly increases the execution time of every stack operation.
 * - Requires a slightly larger amount of memory.
 */
#define CANARY_PROTECT

/** Hash Protection
 * - Strengthens security using a hash function to protect the stack and data.
 * - Significantly increases the execution time of every stack operation.
 */
#undef HASH_PROTECT

// Element type.
typedef Token elem_t;

// Canary type.
typedef unsigned long long canary_t;

// Format specifier for printf() to print elements.
#define ELEM_FORMAT "\0"

// Poison value for uninitialized elements.
static const elem_t POISON = {};

// Format specifier for printf() to print canary values.
#define CANARY_FORMAT "%llx"

// Canary value to protect data.
static const canary_t CANARY_VALUE = 0xBAADF00D;

// Default minimum capacity that the stack can have.
static const int STACK_SIZE_DEFAULT = 64;

// Multiplier by which the capacity of the stack will be increased when needed.
static const float STACK_CAPACITY_MULTIPLIER = 2.0;

#endif