#include<main.h>

#ifndef TERMINAL_H
#define TERMINAL_H
    void enableRawMode(void); // enabling raw mode
    void disableRawMode(void); //disabling raw mode after program is exited;
    void die(const char*); // for error handling
    int editorReadKey(void); // waits for a keypress and return it
    int getWindowSize(int*,int*);
    int getCursorPosition(int*,int*);
#endif
