#include<editor_operations.h>
#include<row_operations.h>
#include<init.h>
#include<input.h>
#include<utils.h>
void editorInsertNewline(){
    if(E.cx == 0){
        editorInsertRow(E.cy,"",0);
        E.cx = 0;
    }else{
        erow *row = &E.row[E.cy];
        int len = 0;            // len of buffer created
        int newCx = 0;          // location of new Cx
        
        char *buf = prependSpaces(E.cy,E.cx,&len,&newCx);
        editorInsertRow(E.cy+1,buf,len);
        free(buf);

        row = &E.row[E.cy]; // reassign since the above fxn calls realloc which might change the 
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editorUpdateRow(row);
        E.cx = newCx;
    }
    E.cy++;
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
