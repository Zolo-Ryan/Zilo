#include<main.h>

#ifndef BUFFER_H
#define BUFFER_H
    #define ABUF_INIT {NULL,0}
    struct abuf{
        char *b;
        int len;
    };
    void abAppend(struct abuf*,const char*,int);
    void abFree(struct abuf*);
#endif