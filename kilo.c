/* includes */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<termios.h>
#include<ctype.h>
#include<sys/ioctl.h>

/* defines */
#define KILO_VERSION "0.0.1"
#define CTRL_KEY(k) ((k) & 0x1f)
enum editorKey{
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN
};

/* data */
struct editorConfig{
    int cx, cy;
    int screenrows;
    int screencols;
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

/* input */
void editorProcessKeypress(void); // waits for a keypress and then handles it
void editorMoveCursor(int);

/* init */
void initEditor(void);

int main()
{
    enableRawMode();
    initEditor();

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}

/** init **/
void initEditor(){
    E.cx = 0; // col(horizontal coordinate)
    E.cy = 0; // row(vertical coordinate)

    if(getWindowSize(&E.screenrows,&E.screencols) == -1) die("getWindowSize");
}

/** output **/
void editorRefreshScreen(){
    struct abuf ab = ABUF_INIT;
    abAppend(&ab,"\x1b[?25l",6); // hides the cursor
    // abAppend(&ab,"\x1b[2J",4); // \x1b is escape(27). escape sequence start with '<esc>[', here we are running J command and giving a parameter of '2', hence 4 bits
    abAppend(&ab,"\x1b[H",3); // takes two arguments, 80x24 h toh <esc>[12;40H for center
    
    editorDrawRows(&ab);
    
    char buf[32];
    snprintf(buf,sizeof(buf),"\x1b[%d;%dH",E.cy+1,E.cx+1); // moves the cursor to its correct location
    abAppend(&ab,buf,strlen(buf));
    
    abAppend(&ab,"\x1b[?25h",6); // makes the cursor visible

    write(STDOUT_FILENO,ab.b,ab.len);
    abFree(&ab);
}
void editorDrawRows(struct abuf *ab){
    int y;
    for(y = 0;y<E.screenrows;y++){
        if(y == E.screenrows / 3){
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
        abAppend(ab,"\x1b[K",3); // K (erase in line). erases part of current line.2 for whole, 1 for left of cursor, 0 (default) for right
        if(y < E.screenrows - 1){
            abAppend(ab,"\r\n",2);
        }
    }
}

/** input **/
void editorProcessKeypress(){
    int c = editorReadKey();

    switch(c){
        case CTRL_KEY('q'):
            write(STDOUT_FILENO,"\x1b[2J",4); // \x1b is escape(27). escape sequence start with '<esc>[', here we are running J command and giving a parameter of '2', hence 4 bits
            write(STDOUT_FILENO,"\x1b[H",3); // takes two arguments, 80x24 h toh <esc>[12;40H for center
            exit(EXIT_SUCCESS);
            break;
        
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;
    }
}
void editorMoveCursor(int key){
    switch(key){
        case ARROW_LEFT:
            if(E.cx > 0)
                E.cx--;
            break;
        case ARROW_RIGHT:
            if(E.cx < E.screencols - 1)
                E.cx++;
            break;
        case ARROW_UP:
            if(E.cy > 0)
                E.cy--;
            break;
        case ARROW_DOWN:
            if(E.cy < E.screenrows - 1)
                E.cy++;
            break;
    }
}

/** terminal **/
int editorReadKey(){
    int nread;
    char c;
    while((nread = read(STDIN_FILENO,&c,1)) != 1){
        if(nread == -1 && errno != EAGAIN) die("read");
    }

    if(c == '\x1b'){
        char seq[3];
        
        if(read(STDIN_FILENO,&seq[0],1) != 1) return '\x1b'; // => only escape was pressed
        if(read(STDIN_FILENO,&seq[1],1) != 1) return '\x1b'; // => some other escape seq was pressed but we just return escape

        if(seq[0] == '['){
            switch(seq[1]){
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
            }
        }

        return '\x1b'; // <esc>? was pressed. here ? denotes idk what was the character was after <esc> but i am sure it is not [
    }
    else
        return c;
}
void enableRawMode(void){
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
