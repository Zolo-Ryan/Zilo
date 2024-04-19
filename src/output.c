#include<output.h>
#include<highlight.h>
#include<row_operations.h>
#include<buffer.h>
#include<utils.h>

void editorDrawMessageBar(struct abuf *ab){
    abAppend(ab,"\x1b[K",3);
    int msglen = strlen(E.statusmsg);
    if(msglen > E.screencols) msglen = E.screencols;
    if(msglen && time(NULL) - E.statusmsg_time < 5)
        abAppend(ab,E.statusmsg,msglen);
}
void editorSetStatusMessage(const char *fmt,...){
    va_list ap;
    va_start(ap,fmt); // requires the last fixed parameter to get the address
    vsnprintf(E.statusmsg,sizeof(E.statusmsg),fmt,ap); // snprintf but uses ap (variable argument list)
    va_end(ap);
    E.statusmsg_time = time(NULL);
}
void editorDrawStatusBar(struct abuf* ab){
    abAppend(ab,"\x1b[7m",4); // inverts the color, \x1b[m for normal formatting
    
    char status[80],rstatus[80];
    int len = snprintf(status,sizeof(status),"%.20s - %d lines %s",E.filename ? E.filename: "[No Name]", E.numrows,E.dirty ? "(modified)":"");
    int rlen = snprintf(rstatus,sizeof(rstatus),"%s | %d/%d",E.syntax ? E.syntax->filetype:"no ft",E.cy+1,E.numrows);

    if(len > E.screencols) len = E.screencols;
    abAppend(ab,status,len);

    while(len < E.screencols){
        if(E.screencols - len == rlen){
            abAppend(ab,rstatus,rlen);
            break;
        }else{
            abAppend(ab," ",1);
            len++;
        }
    }
    abAppend(ab,"\x1b[m",3); // 1 for bold,3 for italic, 4 for underscore, 5 for blink, 8 for invisible, 9 for strike
    abAppend(ab,"\r\n",2);
}
void editorRefreshScreen(){
    editorScroll();

    E.sidebar_width = digits(E.numrows) + 2;

    struct abuf ab = ABUF_INIT;
    abAppend(&ab,"\x1b[?25l",6); // hides the cursor
    // abAppend(&ab,"\x1b[2J",4); // \x1b is escape(27). escape sequence start with '<esc>[', here we are running J command and giving a parameter of '2', hence 4 bits
    abAppend(&ab,"\x1b[H",3); // takes two arguments, 80x24 h toh <esc>[12;40H for center
    
    editorDrawMenuBar(&ab);
    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);
    editorDrawMessageBar(&ab);
    
    char buf[32];
    snprintf(buf,sizeof(buf),"\x1b[%d;%dH",(E.cy - E.rowoff)+ 1 + MENU_HEIGHT,(E.rx - E.coloff + E.sidebar_width)+1); // moves the cursor to its correct location
    abAppend(&ab,buf,strlen(buf));
    
    abAppend(&ab,"\x1b[?25h",6); // makes the cursor visible

    write(STDOUT_FILENO,ab.b,ab.len);
    abFree(&ab);
}
void editorDrawRows(struct abuf *ab){
    int y;
    for(y = 0;y<E.screenrows;y++){
        int filerow = y + E.rowoff;
        editorDrawSidebar(ab,filerow);
        if(filerow >= E.numrows){
            // draw welcome string if file not opened
            if(E.numrows == 0 && y == E.screenrows / 3){
                char welcome[80];
                int welcomelen = snprintf(welcome,sizeof(welcome),"Kilo editor -- version %s",KILO_VERSION);

                if(welcomelen > E.screencols) welcomelen = E.screencols;
                int padding = (E.screencols - welcomelen) / 2;
                if(padding){
                    abAppend(ab,"~",1);
                    padding--;
                }
                while(padding--) abAppend(ab," ",1);
                abAppend(ab,welcome,welcomelen);
            }
            else{
                abAppend(ab,"~",1);
            }
        }else{
            int len = E.row[filerow].rsize - E.coloff;
            if(len < 0) len = 0;
            if(len > E.screencols - E.sidebar_width) len = E.screencols - E.sidebar_width;
            
            char *c = &E.row[filerow].render[E.coloff];
            unsigned char *hl = &E.row[filerow].hl[E.coloff];

            int current_color = -1;
            int j;
            for(j = 0;j<len;j++){
                if(iscntrl(c[j])){
                    char sym = (c[j] <= 26) ? '@' + c[j]: '?'; // @ABCDEF... in Ascii table
                    abAppend(ab,"\x1b[7m",4);
                    abAppend(ab,&sym,1);
                    abAppend(ab,"\x1b[m",3);
                    if(current_color != -1){
                        char buf[16];
                        int clen = snprintf(buf,sizeof(buf),"\x1b[%dm",current_color);
                        abAppend(ab,buf,clen);
                    }
                }
                else if(hl[j] == HL_NORMAL){
                    if(current_color != -1){
                        abAppend(ab,"\x1b[39m",5);
                        current_color = -1;
                    }
                    abAppend(ab,&c[j],1);
                }else{
                    int color = editorSyntaxToColor(hl[j]);
                    if(color != current_color){
                        char buf[16];
                        int clen = snprintf(buf,sizeof(buf),"\x1b[%dm",color);
                        abAppend(ab,buf,clen);
                        current_color = color;
                    }
                    abAppend(ab,&c[j],1);
                }
            }
            abAppend(ab,"\x1b[39m",5);
        }
        abAppend(ab,"\x1b[K",3); // K (erase in line). erases part of current line.2 for whole, 1 for left of cursor, 0 (default) for right
        abAppend(ab,"\r\n",2);
    }
}
void editorScroll(){
    E.rx = 0;
    if(E.cy < E.numrows){ // idk why added
        E.rx = editorRowCxToRx(&E.row[E.cy],E.cx);
    }

    if(E.cy < E.rowoff){ // for cursor going up
        E.rowoff = E.cy;
    }
    if(E.cy >= E.rowoff + E.screenrows){ // for cursor going down
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if(E.rx < E.coloff){ // for cursor going left
        E.coloff = E.rx;
    }
    if(E.rx >= E.coloff + E.screencols - E.sidebar_width){ // for cursor going right
        E.coloff = E.rx - E.screencols + 1 + E.sidebar_width;
    }
}
void editorDrawSidebar(struct abuf *ab,int num){
    if(num >= E.numrows) return;
    char buf[16];
    int padding = E.sidebar_width - 1 - digits(num+1); // -1 for right padding
    int nlen = snprintf(buf,sizeof(buf),"%d",num+1);
    
    char cbuf[16];
    int clen = snprintf(cbuf,sizeof(cbuf),"\x1b[%dm",editorSyntaxToColor(HL_SIDEBAR));
    abAppend(ab,cbuf,clen);

    while(padding-- > 0)
        abAppend(ab," ",1);
    abAppend(ab,buf,nlen);
    abAppend(ab," ",1);
    abAppend(ab,"\x1b[49m",5);
}
void editorDrawMenuBar(struct abuf *ab){
    if(z.size < 1) return;
    char buf[200];
    int i = 0,j = 0;
    // puts all file name into menu (can't handle files with long names or too many file names)
    while(i < z.size){
        if(j + charptrLen(z.openBuffers[i].filename) >= E.screencols) break;
        if(i == z.currentPointer){
            sprintf(&buf[j],"\x1b[m");
            j += 3;
        }
        
        j += sprintf(&buf[j]," %s ",charptrName(z.openBuffers[i].filename));
        
        if(i == z.currentPointer){
            sprintf(&buf[j],"\x1b[45m");
            j += 5;
        }
        j += sprintf(&buf[j],"|");
        i++;
    }
    buf[j] = '\0';
    abAppend(ab,"\x1b[45m",5);
    abAppend(ab,buf,strlen(buf)+1);
    while(j++ - 8 < E.screencols) // -8 since +5 and +3 due to color change
        abAppend(ab," ",1);
    abAppend(ab,"\r\n",2);
    abAppend(ab,"\x1b[m",3);
}