#include "../include/tree_fileOperations.h"


static TreeError getFileSize(FILE* file, size_t* size)
{
    struct stat bf = {};
    if (fstat(fileno(file), &bf) == -1)
       return TREE_ERROR_BAD_STAT;

    *size = (size_t) bf.st_size;
    return TREE_ERROR_NO;
}


static TreeError putFileToBuffer(char** buffer, const size_t size, FILE* file)
{
    // Size is in BYTES (not the amount of 'char's).
    char* tempBuf = (char*) calloc(size + 1, 1);
    if (tempBuf == NULL)
    {
        return TREE_ERROR_BAD_MEM_ALLOC;
    }

    size_t sizeRef = fread(tempBuf, 1, size, file);
    if (sizeRef != size)
    {
        free(tempBuf);
        fprintf(stderr, "FUCK YOU FREAD\n");
        return TREE_ERROR_BAD_FREAD;
    }

    *buffer = tempBuf;
    (*buffer)[size] = '\0';
    return TREE_ERROR_NO;
}


TreeError createCharBuffer(char** buffer, size_t* size, FILE* file)
{
    size_t bufferSize = 0;
    TreeError fileErr = getFileSize(file, &bufferSize);
    if (fileErr != TREE_ERROR_NO)
        return fileErr;

    fileErr = putFileToBuffer(buffer, bufferSize, file);
    if (fileErr != TREE_ERROR_NO)
        return fileErr;
    (*buffer)[bufferSize] = '\0';
    fprintf(stderr, "bufferSize = %zu\n", bufferSize);

    return TREE_ERROR_NO;
}
