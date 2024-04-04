#include<clipboard.h>
#include<output.h>
#include<init.h>
#include<row_operations.h>

clip clipboard = CLIP_INIT;
void copyToClipboard(){
    int at = E.cy;
    if(at < 0 || at >= E.numrows) return;
    clipboard.str = realloc(clipboard.str,sizeof(char)*(E.row[at].size+1));
    clipboard.len = E.row[at].size;
    memcpy(clipboard.str,E.row[at].chars,E.row[at].size);
    clipboard.str[clipboard.len] = '\0';
    editorSetStatusMessage("Copied to clipboard!");
}
void cutToClipboard(){
    int at = E.cy;
    copyToClipboard();
    editorDelRow(at);
}
void pasteFromClipboard(){
    int at = E.cy;
    editorInsertRow(at,clipboard.str,clipboard.len);
    editorSetStatusMessage("Pasted!");
}