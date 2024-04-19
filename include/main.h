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

#ifndef MAIN_H
#define MAIN_H
    #define KILO_VERSION "1.0.0"
    #define KILO_TAB_STOP 8
    #define KILO_QUIT_TIMES 1
    #define CTRL_KEY(k) ((k) & 0x1f)
    #define HL_HIGHLIGHT_NUMBERS (1<<0)
    #define HL_HIGHLIGHT_STRINGS (1<<1)
    #define MENU_HEIGHT 1
    #define MENU_FILENAME_WIDTH 10
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
    enum editorHighlight{
        HL_NORMAL = 0,
        HL_COMMENT,
        HL_MLCOMMENT,
        HL_KEYWORD1,
        HL_KEYWORD2,
        HL_STRING,
        HL_NUMBER,
        HL_MATCH,
        HL_SIDEBAR
    };

    /* data */
    struct editorSyntax{
        char *filetype;
        char **filematch;
        char **keywords; // | for secondary keywords
        char *singleline_comment_start;
        char *multiline_comment_start;
        char *multiline_comment_end;
        int flags;
    };
    typedef struct erow{
        int idx; // index of a row within file
        int size;
        char *chars; // actual value of a row
        int rsize;
        char *render; // value of the row to be rendered on screen (tab)
        unsigned char *hl; // contains the color to be given to each character in render string
        int hl_open_comment; // boolean to store if row part of unclosed mlcomment or not
    }erow;
    struct editorConfig{
        int cx, cy; // cursor position relative to file not screen
        int rx; // render x
        int rowoff; // row offset to track what row of file the user is currently scrolled to(refers to top)
        int coloff; // col offset to track col
        int screenrows;
        int screencols;
        int numrows;
        int sidebar_width; // sidebar width
        erow *row;
        int dirty; // to track if any changes are made to file
        char *filename;
        char statusmsg[80];
        time_t statusmsg_time;
        struct editorSyntax *syntax;
        struct termios orig_termios;
    };
    
    extern struct editorConfig E;
    /* file types */
    extern char *C_HL_extensions[];
    extern char *C_HL_keywords[];
    extern struct editorSyntax HLDB[];
    extern size_t HLDB_ENTRIES;
#endif