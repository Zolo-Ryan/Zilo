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
#define CTRL_KEY(k) ((k) & 0x1f)

/* data */
struct editorConfig{
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
char editorReadKey(void); // waits for a keypress and return it
int getWindowSize(int*,int*);
int getCursorPosition(int*,int*);

/* output */
void editorRefreshScreen(void); // to refresh the screen initially
void editorDrawRows(struct abuf *ab); // to draw tilde on the side of screen just like vim does

/* input */
void editorProcessKeypress(void); // waits for a keypress and then handles it

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


void initEditor(){
    if(getWindowSize(&E.screenrows,&E.screencols) == -1) die("getWindowSize");
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

void editorRefreshScreen(){
    struct abuf ab = ABUF_INIT;
    abAppend(&ab,"\x1b[2J",4); // \x1b is escape(27). escape sequence start with '<esc>[', here we are running J command and giving a parameter of '2', hence 4 bits
    abAppend(&ab,"\x1b[H",3); // takes two arguments, 80x24 h toh <esc>[12;40H for center
    editorDrawRows(&ab);
    abAppend(&ab,"\x1b[H",3);

    write(STDOUT_FILENO,ab.b,ab.len);
    abFree(&ab);
}
void editorDrawRows(struct abuf *ab){
    int y;
    for(y = 0;y<E.screenrows;y++){
        abAppend(ab,"~",1);
        if(y < E.screenrows - 1){
            abAppend(ab,"\r\n",2);
        }
    }
}

void editorProcessKeypress(){
    char c = editorReadKey();

    switch(c){
        case CTRL_KEY('q'):
            write(STDOUT_FILENO,"\x1b[2J",4); // \x1b is escape(27). escape sequence start with '<esc>[', here we are running J command and giving a parameter of '2', hence 4 bits
            write(STDOUT_FILENO,"\x1b[H",3); // takes two arguments, 80x24 h toh <esc>[12;40H for center
            exit(EXIT_SUCCESS);
            break;
    }
}
char editorReadKey(){
    int nread;
    char c;
    while((nread = read(STDIN_FILENO,&c,1)) != 1){
        if(nread == -1 && errno != EAGAIN) die("read");
    }
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
