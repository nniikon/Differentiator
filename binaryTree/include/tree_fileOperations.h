#ifndef TREE_FILE_OPERATIONS_H_
#define TREE_FILE_OPERATIONS_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "tree.h"

TreeError createCharBuffer(char** buffer, size_t* size, FILE* file);

#endif // TREE_FILE_OPERATIONS_H_