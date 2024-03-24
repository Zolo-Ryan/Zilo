/* includes */
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<termios.h>
#include<ctype.h>
#include<stdlib.h>
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

/* terminal */
void enableRawMode(void); // enabling raw mode
void disableRawMode(void); //disabling raw mode after program is exited;
void die(const char*); // for error handling
char editorReadKey(void); // waits for a keypress and return it
int getWindowSize(int*,int*);

/* output */
void editorRefreshScreen(void); // to refresh the screen initially
void editorDrawRows(void); // to draw tilde on the side of screen just like vim does

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

int getWindowSize(int *rows,int *cols){
    struct winsize ws;

    // input output control on stdout, terminal input output control get window size, structure
    if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws) == -1 || ws.ws_col == 0)
        return -1;
    else{
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

void editorRefreshScreen(){
    write(STDOUT_FILENO,"\x1b[2J",4); // \x1b is escape(27). escape sequence start with '<esc>[', here we are running J command and giving a parameter of '2', hence 4 bits
    write(STDOUT_FILENO,"\x1b[H",3); // takes two arguments, 80x24 h toh <esc>[12;40H for center
    editorDrawRows();
    write(STDOUT_FILENO,"\x1b[H",3);
}
void editorDrawRows(){
    int y;
    for(y = 0;y<E.screenrows;y++){
        write(STDOUT_FILENO,"~\r\n",3);
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
