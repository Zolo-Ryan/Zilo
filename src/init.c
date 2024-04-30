#include<init.h>
#include<terminal.h>
#include<zBuffer.h>

char *C_HL_extensions[] = {".c", ".h", ".cpp", NULL};

char *C_HL_keywords[] = {
    "auto|", "break", "case", "char", "const|", "continue", "default", "do",
    "double|", "else", "enum|", "extern", "float|", "for", "goto", "if",
    "inline", "int|", "long|", "register|", "return", "short|", "signed|",
    "sizeof", "static|", "struct|", "switch", "typedef|", "union|", "unsigned|",
    "void|", "volatile", "while",
    // Preprocessor directives
    "#define", "#elif", "#else", "#endif", "#error", "#if", "#ifdef",
    "#ifndef", "#include", "#line", "#pragma", "#undef",
    // Standard library functions
    "abort", "abs", "acos", "asctime", "asin", "assert", "atan",
    "atan2", "atexit", "atof", "atoi", "atol", "bsearch", "calloc",
    "ceil", "clearerr", "clock", "cos", "cosh", "ctime", "difftime",
    "div", "exit", "exp", "fabs", "fclose", "feof", "ferror", "fflush",
    "fgetc", "fgetpos", "fgets", "fopen", "fprintf", "fputc", "fputs",
    "fread", "free", "freopen", "frexp", "fscanf", "fseek", "fsetpos",
    "ftell", "fwrite", "getc", "getchar", "getenv", "gets", "gmtime",
    "isalnum", "isalpha", "iscntrl", "isdigit", "isgraph", "islower",
    "isprint", "ispunct", "isspace", "isupper", "isxdigit", "labs",
    "ldexp", "ldiv", "localeconv", "localtime", "log", "log10", "longjmp",
    "malloc", "mblen", "mbstowcs", "mbtowc", "memchr", "memcmp", "memcpy",
    "memmove", "memset", "mktime", "modf", "perror", "pow", "printf",
    "putc", "putchar", "puts", "qsort", "raise", "rand", "realloc", "remove",
    "rename", "rewind", "scanf", "setbuf", "setjmp", "setlocale", "setvbuf",
    "signal", "sin", "sinh", "sprintf", "sqrt", "srand", "sscanf", "strcat",
    "strchr", "strcmp", "strcoll", "strcpy", "strcspn", "strerror",
    "strftime", "strlen", "strncat", "strncmp", "strncpy", "strpbrk",
    "strrchr", "strspn", "strstr", "strtod", "strtok", "strtol", "strtoul",
    "strxfrm", "system", "tan", "tanh", "time", "tmpfile", "tmpnam", "tolower",
    "toupper", "ungetc", "va_arg", "va_end", "va_start", "vfprintf", "vprintf",
    "vsprintf", NULL
};


char *Java_HL_extensions[] = {".java", NULL};
char *Java_HL_keywords[] = {
    "abstract", "assert", "boolean", "break", "byte|", "case",
    "catch", "char|", "class", "const", "continue", "default",
    "do", "double", "else", "enum", "extends", "final", "finally",
    "float|", "for", "goto", "if", "implements", "import",
    "instanceof", "int|", "interface", "long|", "native", "new",
    "package", "private", "protected", "public", "return",
    "short|", "static", "strictfp", "super", "switch", "synchronized",
    "this", "throw", "throws", "transient", "try", "void|", "volatile",
    "while", NULL
};


struct editorSyntax HLDB[] = {
    {"c",C_HL_extensions,C_HL_keywords,"//","/*","*/",HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},
    {"java",Java_HL_extensions,Java_HL_keywords,"//","/*","*/",HL_HIGHLIGHT_NUMBERS |HL_HIGHLIGHT_STRINGS},
};
zBuffer z = {NULL,0,-1};

/* add java here

*/
size_t HLDB_ENTRIES = (int)(sizeof(HLDB) / sizeof(HLDB[0]));
struct editorConfig E;
void initEditor(){
    E.cx = 0; // col(horizontal coordinate in file)
    E.cy = 0; // row(vertical coordinate in file)
    E.rx = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.sidebar_width = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.syntax = NULL;

    if(getWindowSize(&E.screenrows,&E.screencols) == -1) die("getWindowSize");
    E.screenrows -= 3;
}
