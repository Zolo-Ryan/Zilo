#include<editor_operations.h>
#include<row_operations.h>
#include<init.h>
#include<input.h>

void editorInsertNewline(){
    if(E.cx == 0){
        editorInsertRow(E.cy,"",0);
    }else{
        erow *row = &E.row[E.cy];
        editorInsertRow(E.cy+1,&row->chars[E.cx],row->size - E.cx);
        row = &E.row[E.cy]; // reassign since the above fxn calls realloc which might change the 
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editorUpdateRow(row);
    }
    E.cy++;
    E.cx = 0;
}
void editorDelChar(){
    if(E.cy == E.numrows) return;
    if(E.cx == 0 && E.cy == 0)return;

    erow *row = &E.row[E.cy];
    if(E.cx > 0){
        editorRowDelChar(row,E.cx - 1);
        E.cx--;
    }else{ // E.cx <=0
        E.cx = E.row[E.cy - 1].size;
        editorRowAppendString(&E.row[E.cy-1],row->chars,row->size);
        editorDelRow(E.cy);
        E.cy--;
    }
}
void editorInsertChar(int c){
    if(E.cy == E.numrows){
        editorInsertRow(E.numrows,"",0); // create a new empty row
    }
    editorRowInsertChar(&E.row[E.cy],E.cx,c);
    E.cx++;
}
void goToLineCallback(char *query,int key){
    int cy;
    sscanf(query,"%d",&cy);
    if(cy <= E.numrows+1 && cy >= 1)
        E.cy = cy - 1;
}
void goToLine(){
    int saved_cy = E.cy;

    char *query = editorPrompt("Enter line number: %s",goToLineCallback);
    if(query){    
        free(query);
    }else {
        E.cy = saved_cy;
    }
}
