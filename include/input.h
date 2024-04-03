#include<main.h>

#ifndef INPUT_H
#define INPUT_H
    void editorProcessKeypress(void); // waits for a keypress and then handles it
    void editorMoveCursor(int);
    char *editorPrompt(char*,void (*callback)(char*,int));
#endif