/* includes */
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<termios.h>
#include<ctype.h>
#include<stdlib.h>

/* defines */
#define CTRL_KEY(k) ((k) & 0x1f)

/* data */
struct termios orig_termios;

/* terminal */
void enableRawMode(void); // enabling raw mode
void disableRawMode(void); //disabling raw mode after program is exited;
void die(const char*); // for error handling
char editorReadKey(void); // waits for a keypress and return it

/* output */
void editorRefreshScreen(void); // to refresh the screen initially

/* input */
void editorProcessKeypress(void); // waits for a keypress and then handles it

int main()
{
    enableRawMode();

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
  }
    return 0;
}

void editorRefreshScreen(){
    write(STDOUT_FILENO,"\x1b[2J",4); // \x1b is escape(27). escape sequence start with '<esc>[', here we are running J command and giving a parameter of '2', hence 4 bits
    write(STDOUT_FILENO,"\x1b[H",3); // takes two arguments, 80x24 h toh <esc>[12;40H for center
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
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = orig_termios;
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
    if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&orig_termios) == -1)
        die("tcsetattr");
}

void die(const char* message){
    write(STDOUT_FILENO,"\x1b[2J",4); // \x1b is escape(27). escape sequence start with '<esc>[', here we are running J command and giving a parameter of '2', hence 4 bits
    write(STDOUT_FILENO,"\x1b[H",3); // takes two arguments, 80x24 h toh <esc>[12;40H for center
    perror(message);
    exit(EXIT_FAILURE);
}
