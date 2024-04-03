#include<terminal.h>

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
    E.rx -= E.sidebar_width;
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
