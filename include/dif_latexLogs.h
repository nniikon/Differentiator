#ifndef DIF_LATEX_LOGS_H_
#define DIF_LATEX_LOGS_H_

#include <stdio.h>
#include "dif.h"

FILE* difLatexOpenFile(const char* filePath);

void difLatexCompileFile(FILE* file, const char* filePath,
                                     const char* outputPath);

void difLatexDumpTree(Dif* dif, Tree* tree);

void difLatexGenGraph();

#endif // DIF_LATEX_LOGS_H_