#include<main.h>

#ifndef CLIPBOARD_H
#define CLIPBOARD_H
    #define CLIP_INIT {NULL,0}
    typedef struct clip{
        char *str;
        int len;
    } clip;
    extern clip clipboard;
    void copyToClipboard(void);
    void cutToClipboard(void);
    void pasteFromClipboard(void);
#endif