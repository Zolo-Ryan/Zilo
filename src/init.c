#include<init.h>
#include<terminal.h>

char *C_HL_extensions[] = {".c",".h",".cpp",NULL};
char *C_HL_keywords[] = {"switch","if","while","for","break",
"continue","return","struct","union","typedef","static","enum","class","case",
"int|","long|","double|","float|","char|","unsigned|","void|",NULL};
struct editorSyntax HLDB[] = {
    {"c",C_HL_extensions,C_HL_keywords,"//","/*","*/",HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},
};
size_t HLDB_ENTRIES = (int)(sizeof(HLDB) / sizeof(HLDB[0]));
struct editorConfig E;
void initEditor(){
    E.cx = 0; // col(horizontal coordinate in file)
    E.cy = 0; // row(vertical coordinate in file)
    E.rx = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.syntax = NULL;

    if(getWindowSize(&E.screenrows,&E.screencols) == -1) die("getWindowSize");
    E.screenrows -= 2;
}