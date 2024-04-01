#include<main.h>

#ifndef ROW_OPERATIONS_H
#define ROW_OPERATIONS_H
    void editorInsertRow(int,char*,size_t);
    void editorUpdateRow(erow*); // uses chars string to fill the contents in render string
    int editorRowCxToRx(erow*,int); // converts cx to rx
    void editorRowInsertChar(erow*,int,int);
    void editorRowDelChar(erow*,int);
    void editorFreeRow(erow*);
    void editorDelRow(int);
    void editorRowAppendString(erow*,char*,size_t);
    int editorRowRxToCx(erow*,int); // converts rx to cx
#endif