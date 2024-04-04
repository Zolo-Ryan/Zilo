#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE
// the above defines must come before any includes

#include<main.h>

#ifndef FILE_IO_H
#define FILE_IO_H
    void editorOpen(char*);
    char *editorRowsToString(int*);
    void editorSave(void);
#endif