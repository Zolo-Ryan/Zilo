#include<main.h>
#include<buffer.h>

#ifndef OUTPUT_H
#define OUTPUT_H
    void editorRefreshScreen(void); // to refresh the screen initially
    void editorDrawRows(struct abuf *ab); // to draw tilde on the side of screen just like vim does
    void editorScroll(void);
    void editorDrawStatusBar(struct abuf*);
    void editorDrawMenuBar(struct abuf*);
    void editorSetStatusMessage(const char*,...);
    void editorDrawMessageBar(struct abuf*);
    void editorDrawSidebar(struct abuf*,int);
#endif