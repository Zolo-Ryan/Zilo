#include<main.h>

#ifndef EDITOR_OPERATIONS_H
#define EDITOR_OPERATIONS_H
    void editorInsertChar(int);
    void editorDelChar(void);
    void editorInsertNewline(void);
    void goToLine(void);
    void goToLineCallback(char*,int);
#endif