#include<main.h>

#ifndef Z_BUFFER_H
#define Z_BUFFER_H
    typedef struct zBuffer{
        struct editorConfig *openBuffers;
        int size;
        int currentPointer;
    }zBuffer;
    extern zBuffer z;
    void switchBuffer(int);
    void initZBuffer(int,char**);
#endif