/* includes */
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE
// the above defines must come before any includes
#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<termios.h>
#include<ctype.h>
#include<stdarg.h>
#include<time.h>
#include<sys/ioctl.h>

/* defines */
#define KILO_VERSION "0.0.1"
#define KILO_TAB_STOP 8
#define KILO_QUIT_TIMES 3
#define CTRL_KEY(k) ((k) & 0x1f)
enum editorKey{
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

/* data */
typedef struct erow{
    int size;
    char *chars;
    int rsize;
    char *render;
}erow;
struct editorConfig{
    int cx, cy; // cursor position relative to file not screen
    int rx; // render x
    int rowoff; // row offset to track what row of file the user is currently scrolled to(refers to top)
    int coloff; // col offset to track col
    int screenrows;
    int screencols;
    int numrows;
    erow *row;
    int dirty; // to track if any changes are made to file
    char *filename;
    char statusmsg[80];
    time_t statusmsg_time;
    struct termios orig_termios;
};
struct editorConfig E;

/* append buffer */
#define ABUF_INIT {NULL,0}
struct abuf{
    char *b;
    int len;
};
void abAppend(struct abuf *ab,const char *s,int len){
    char *new = realloc(ab->b,ab->len + len);

    if(new == NULL) return;
    memcpy(&new[ab->len],s,len);
    ab->b = new;
    ab->len += len;
}
void abFree(struct abuf *ab){
    free(ab->b);
}

/* terminal */
void enableRawMode(void); // enabling raw mode
void disableRawMode(void); //disabling raw mode after program is exited;
void die(const char*); // for error handling
int editorReadKey(void); // waits for a keypress and return it
int getWindowSize(int*,int*);
int getCursorPosition(int*,int*);

/* output */
void editorRefreshScreen(void); // to refresh the screen initially
void editorDrawRows(struct abuf *ab); // to draw tilde on the side of screen just like vim does
void editorScroll(void);
void editorDrawStatusBar(struct abuf*);
void editorSetStatusMessage(const char*,...);
void editorDrawMessageBar(struct abuf*);

/* input */
void editorProcessKeypress(void); // waits for a keypress and then handles it
void editorMoveCursor(int);
char *editorPrompt(char*,void (*callback)(char*,int));

/* init */
void initEditor(void);

/* file i/o */
void editorOpen(char*);
char *editorRowsToString(int*);
void editorSave(void);

/* row operations */
void editorInsertRow(int,char*,size_t);
void editorUpdateRow(erow*); // uses chars string to fill the contents in render string
int editorRowCxToRx(erow*,int); // converts cx to rx
void editorRowInsertChar(erow*,int,int);
void editorRowDelChar(erow*,int);
void editorFreeRow(erow*);
void editorDelRow(int);
void editorRowAppendString(erow*,char*,size_t);
int editorRowRxToCx(erow*,int); // converts rx to cx

/* editor operations */
void editorInsertChar(int);
void editorDelChar(void);
void editorInsertNewline(void);

/* find */
void editorFind(void);
void editorFindCallback(char*,int);

int main(int argc, char *argv[])
{
    enableRawMode();
    initEditor();
    if(argc >= 2){
        editorOpen(argv[1]);
    }

    editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}

/** find **/
void editorFindCallback(char *query,int key){
    static int last_match = -1;
    static int direction = 1;

    if(key == '\r' || key == '\x1b'){
        last_match = -1;
        direction = 1; // resetting else have prev value in next search
    }else if(key == ARROW_RIGHT || key == ARROW_DOWN){
        direction = 1;
    }else if(key == ARROW_LEFT || key == ARROW_UP){
        direction = -1;
    }else{
        last_match = -1;
        direction = 1; // reset them since the key pressed is a character and no need to keep track of prev and direction
    }

    if(last_match == -1) direction = 1; // search fwd
    int current = last_match;
    int i;
    for(i = 0;i<E.numrows;i++){
        current += direction;
        if(current == -1) current = E.numrows - 1;
        else if(current == E.numrows) current = 0;

        erow *row = &E.row[current];
        char *match = strstr(row->render,query);
        if(match){
            last_match = current;
            E.cy = current;
            E.cx = editorRowRxToCx(row,match - row->render);
            E.rowoff = E.numrows;
            break;
        }
    }
}
void editorFind(){
    int saved_cx = E.cx;
    int saved_cy = E.cy;
    int saved_coloff = E.coloff;
    int saved_rowoff = E.rowoff;

    char *query = editorPrompt("Search: %s (Use ESC/Arrows/Enter)",editorFindCallback);
    
    if(query){
        free(query);
    }else{
        E.cx = saved_cx;
        E.cy = saved_cy;
        E.coloff = saved_coloff;
        E.rowoff = saved_rowoff;
    }
}

/** editor operations **/
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

/** row operations **/
int editorRowRxToCx(erow *row, int rx){
    int cur_rx = 0;
    int cx;
    for(cx = 0;cx < row->size;cx++){
        if(row->chars[cx] == '\t')
            cur_rx += (KILO_TAB_STOP-1) - (cur_rx % KILO_TAB_STOP);
        cur_rx++;
        if(cur_rx > rx) return cx;
    }
    return cx;
}
void editorRowAppendString(erow *row,char *s,size_t len){
    row->chars = realloc(row->chars,row->size + len + 1);
    memcpy(&row->chars[row->size],s,len);
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.dirty++;
}
void editorDelRow(int at){
    if(at < 0 || at >= E.numrows) return;
    editorFreeRow(&E.row[at]);
    memmove(&E.row[at],&E.row[at+1],sizeof(erow)*(E.numrows - at - 1));
    E.numrows--;
    E.dirty++;
}
void editorFreeRow(erow *row){
    free(row->render);
    free(row->chars);
}
void editorRowDelChar(erow *row,int at){
    if(at < 0 || at >= row->size) return;
    memmove(&row->chars[at],&row->chars[at+1],row->size - at);
    row->size--;
    editorUpdateRow(row);
    E.dirty++;
}
void editorRowInsertChar(erow *row,int at,int c){
    if(at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars,row->size + 2); // shyd 1 hona chaiye
    memmove(&row->chars[at+1],&row->chars[at],row->size - at + 1); // safe to use instead of memcpy when dest and src arrays overlap
    row->size++;
    row->chars[at] = c;
    editorUpdateRow(row); // so that render and rsize fields get updated
    E.dirty++;
}
int editorRowCxToRx(erow *row,int cx){
    int rx = 0;
    int j;
    for(j = 0;j < cx;j++){
        if(row->chars[j] == '\t')
            rx += (KILO_TAB_STOP - 1) - (rx % KILO_TAB_STOP);
        rx++;
    }
    return rx;
}
void editorUpdateRow(erow *row){
    int tabs = 0;
    int j;
    for(j = 0;j< row->size;j++)
        if(row->chars[j] == '\t') tabs++;
    
    free(row->render);
    row->render = malloc(row->size + tabs*7 + 1);

    int idx = 0;
    for(j = 0;j < row->size;j++){
        if(row->chars[j] == '\t'){
            row->render[idx++] = ' ';
            while(idx % KILO_TAB_STOP != 0) row->render[idx++] = ' ';
        }else{
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;
}
void editorInsertRow(int at,char *s,size_t len){
    if(at < 0 || at > E.numrows) return;

    E.row = realloc(E.row,sizeof(erow)*(E.numrows+1));
    memmove(&E.row[at+1],&E.row[at],sizeof(erow)*(E.numrows-at));

    E.row[at].size = len;
    E.row[at].chars = malloc(len+1);
    memcpy(E.row[at].chars,s,len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    editorUpdateRow(&E.row[at]);

    E.numrows++;
    E.dirty++;
}

/** file i/o **/
void editorSave(void){
    if(E.filename == NULL){
        E.filename = editorPrompt("Save as: %s",NULL);
        if(E.filename == NULL){
            editorSetStatusMessage("Save aborted");
            return;
        }
    }

    int len;
    char *buf = editorRowsToString(&len);
    int fd = open(E.filename,O_RDWR | O_CREAT, 0644); // O_TRUNC truncates the file completely, making it an empty file
    if(fd != -1){
        if(ftruncate(fd,len) != -1){
            if(write(fd,buf,len) == len){
                close(fd);
                free(buf);
                editorSetStatusMessage("%d bytes written to disk",len);
                E.dirty = 0; // after saving file is clean
                return;
            }
        }
        close(fd);
    }
    free(buf);
    editorSetStatusMessage("Can't Save! I/O error: %s",strerror(errno));
    // advanced editors will write to a new temporary file and then rename that file to the actual file the user wants to overwrite
}
char *editorRowsToString(int *buflen){
    int totlen = 0;
    int j;
    for(j = 0;j<E.numrows;j++)
        totlen += E.row[j].size + 1;
    *buflen = totlen;
    char *buf = malloc(totlen);
    char *p = buf;
    for(j = 0;j<E.numrows;j++){
        memcpy(p,E.row[j].chars,E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}
void editorOpen(char *filename){
    free(E.filename);
    E.filename = strdup(filename); // makes a copy of string, allocating req memory and assuming you will free that memory

    FILE *fp = fopen(filename,"r");
    if(!fp) die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    // buffer, buffer len, ptr. buffer is allocated memory automatically and len is updated as well
    while((linelen = getline(&line,&linecap,fp)) != -1){ // -1 at EOF
        while(linelen > 0 && (line[linelen-1] == '\n' || line[linelen-1] == '\r'))
            linelen--; // truncating \r\n from back of string
        editorInsertRow(E.numrows,line,linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
}

/** init **/
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

    if(getWindowSize(&E.screenrows,&E.screencols) == -1) die("getWindowSize");
    E.screenrows -= 2;
}

/** output **/
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
    int rlen = snprintf(rstatus,sizeof(rstatus),"%d/%d",E.cy+1,E.numrows);

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

    struct abuf ab = ABUF_INIT;
    abAppend(&ab,"\x1b[?25l",6); // hides the cursor
    // abAppend(&ab,"\x1b[2J",4); // \x1b is escape(27). escape sequence start with '<esc>[', here we are running J command and giving a parameter of '2', hence 4 bits
    abAppend(&ab,"\x1b[H",3); // takes two arguments, 80x24 h toh <esc>[12;40H for center
    
    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);
    editorDrawMessageBar(&ab);
    
    char buf[32];
    snprintf(buf,sizeof(buf),"\x1b[%d;%dH",(E.cy - E.rowoff)+1,(E.rx - E.coloff)+1); // moves the cursor to its correct location
    abAppend(&ab,buf,strlen(buf));
    
    abAppend(&ab,"\x1b[?25h",6); // makes the cursor visible

    write(STDOUT_FILENO,ab.b,ab.len);
    abFree(&ab);
}
void editorDrawRows(struct abuf *ab){
    int y;
    for(y = 0;y<E.screenrows;y++){
        int filerow = y + E.rowoff;
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
            if(len > E.screencols) len = E.screencols;
            abAppend(ab,&E.row[filerow].render[E.coloff],len);
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
    if(E.rx >= E.coloff + E.screencols){ // for cursor going right
        E.coloff = E.rx - E.screencols + 1;
    }
}

/** input **/
char *editorPrompt(char *prompt, void (*callback)(char*,int)){
    size_t bufsize = 128;
    char *buf = malloc(bufsize);

    size_t buflen = 0;
    buf[0] ='\0';

    while(1){
        editorSetStatusMessage(prompt,buf);
        editorRefreshScreen();

        int c = editorReadKey();
        if(c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE){
            if(buflen != 0) buf[--buflen] = '\0';
        }
        else if(c == '\x1b'){
            editorSetStatusMessage("");
            if(callback) callback(buf,c);
            free(buf);
            return NULL;
        }
        else if(c == '\r'){
            if(buflen != 0){
                editorSetStatusMessage("");
                if(callback) callback(buf,c);
                return buf;
            }
        }else if(!iscntrl(c) && c < 128){
            if(buflen == bufsize - 1){
                bufsize *= 2;
                buf = realloc(buf,bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }
        if(callback) callback(buf,c);
    }
}
void editorProcessKeypress(){
    static int quit_times = KILO_QUIT_TIMES;
    int c = editorReadKey();

    switch(c){
        case '\r':
            editorInsertNewline();
            break;
        case CTRL_KEY('q'):
            if(E.dirty && quit_times > 0){
                editorSetStatusMessage("WARNING!!! File has unsaved changes. Press Ctrl-Q %d more times to quit.",quit_times);
                quit_times--;
                return;
            }
            write(STDOUT_FILENO,"\x1b[2J",4); // \x1b is escape(27). escape sequence start with '<esc>[', here we are running J command and giving a parameter of '2', hence 4 bits
            write(STDOUT_FILENO,"\x1b[H",3); // takes two arguments, 80x24 h toh <esc>[12;40H for center
            exit(EXIT_SUCCESS);
            break;
        
        case CTRL_KEY('s'):
            editorSave();
            break;
        
        case HOME_KEY:
            E.cx = 0;
            break;
        case END_KEY:
            if(E.cy < E.numrows)
                E.cx = E.row[E.cy].size;
            break;

        case CTRL_KEY('f'):
            editorFind();
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if(c == DEL_KEY) editorMoveCursor(ARROW_RIGHT);
            editorDelChar();
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                if(c == PAGE_UP){
                    E.cy = E.rowoff;
                }else if(c == PAGE_DOWN){
                    E.cy = E.rowoff + E.screenrows - 1;
                    if(E.cy > E.numrows) E.cy = E.numrows;
                }

                int times = E.screenrows; // can't declare variables directly inside case blocks
                while(times--) editorMoveCursor(c == PAGE_UP ? ARROW_UP: ARROW_DOWN);
            }
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;
        
        case CTRL_KEY('l'): // for screen refresh on terminal
        case '\x1b':
            break;
        
        default:
            editorInsertChar(c);
            break;
    }

    quit_times = KILO_QUIT_TIMES;
}
void editorMoveCursor(int key){
    erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    switch(key){
        case ARROW_LEFT:
            if(E.cx > 0)
                E.cx--;
            else if(E.cy > 0){ // moves cursor to prev line's end if left arrow pressed at starting character
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            break;
        case ARROW_RIGHT:
            if(row && E.cx < row->size){
                E.cx++;
            }else if(row && E.cx == row->size){
                E.cy++;
                E.cx = 0;
            }
            break;
        case ARROW_UP:
            if(E.cy > 0)
                E.cy--;
            break;
        case ARROW_DOWN:
            if(E.cy < E.numrows)
                E.cy++;
            break;
    }

    // if E.cy changed in above switch then this row will be diff than previously dec row
    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if(E.cx > rowlen){
        E.cx = rowlen;
    }
}

/** terminal **/
int editorReadKey(){
    int nread;
    char c;
    while((nread = read(STDIN_FILENO,&c,1)) != 1){
        if(nread == -1 && errno != EAGAIN) die("read");
    }

    /** escape sequences **/
    // ARROW UP,DOWN,RIGHT,BOTTOM => esc[A,esc[B,esc[C,esc[D
    // PAGE UP,DOWN => esc[5~,esc[6~
    // KEY HOME,END => (esc[1~,esc[7~,esc[H,escOH),(esc[4~,esc[8~,esc[F,escOF)
    if(c == '\x1b'){
        char seq[3];
        
        if(read(STDIN_FILENO,&seq[0],1) != 1) return '\x1b'; // => only escape was pressed
        if(read(STDIN_FILENO,&seq[1],1) != 1) return '\x1b'; // => some other escape seq was pressed but we just return escape

        if(seq[0] == '['){
            if(seq[1] >= '0' && seq[1] <= '9'){
                if(read(STDIN_FILENO,&seq[2],1) != 1) return '\x1b'; // => <esc>[\d? type form
                if(seq[2] == '~'){
                    switch(seq[1]){
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            }else{
                switch(seq[1]){
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'F': return END_KEY;
                    case 'H': return HOME_KEY;
                }
            }
        }else if(seq[0] == 'O'){
            switch(seq[1]){
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }

        return '\x1b'; // <esc>? was pressed. here ? denotes idk what was the character was after <esc> but i am sure it is not [
    }
    else
        return c;
}
void enableRawMode(){
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP);
    raw.c_cflag |= (CS8);

    raw.c_iflag &= ~(IXON | ICRNL); // input flag, XON to turn off output flow(s,q), turns off carriage return fixes m
    raw.c_oflag &= ~(OPOST); // turns of translation on the output side: carriage return won't automatically be added
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); // local flags, turn of echo, canonical or cooked mode(\n req for input to be passed into program), turn off v, SIGINT and SIGTSTP signals(c,z)
    raw.c_cc[VMIN] = 0; // minimum number of bytes needed before read can return
    raw.c_cc[VTIME] = 1; // max amt of time to wait before read returns. 1 => 1/10th of a second

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}
void disableRawMode(void){
    if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&E.orig_termios) == -1)
        die("tcsetattr");
}
void die(const char* message){
    write(STDOUT_FILENO,"\x1b[2J",4); // \x1b is escape(27). escape sequence start with '<esc>[', here we are running J command and giving a parameter of '2', hence 4 bits
    write(STDOUT_FILENO,"\x1b[H",3); // takes two arguments, 80x24 h toh <esc>[12;40H for center
    perror(message);
    exit(EXIT_FAILURE);
}
int getCursorPosition(int *rows,int *cols){
    char buf[32];
    unsigned int i = 0;

    if(write(STDOUT_FILENO,"\x1b[6n",4) != 4) return -1; // gives the cursor position which we will read from the stdin
    
    while(i < sizeof(buf) - 1){
        if(read(STDIN_FILENO,&buf[i],1) != 1) break;
        if(buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if(buf[0] != '\x1b' || buf[1] != '[') return -1;
    if(sscanf(&buf[2],"%d;%d",rows,cols) != 2) return -1;
    return 0;
}
int getWindowSize(int *rows,int *cols){
    struct winsize ws;

    // input output control on stdout, terminal input output control get window size, structure
    if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws) == -1 || ws.ws_col == 0){
        if(write(STDOUT_FILENO,"\x1b[999C\x1b[999B",12) != 12) return -1; // C command moves cursor to right, B moves to bottom
        return getCursorPosition(rows,cols);
    }
    else{
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}
