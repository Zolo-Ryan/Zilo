#include<utils.h>

int is_seperator(int c){
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];",c) != NULL; // strchr looks for first occurence of c in str and returns the pointer to it
}
int is_open_character(int c){
    if(c == '(') return ')';
    if(c == '[') return ']';
    if(c == '{') return '}';
    if(c == '\'' || c == '\"') return c;
    return -1;
}
int digits(int a){
    int count = 0;
    while(a > 0){
        count++;
        a /= 10;
    }
    return count;
}

/* Checks if all files are clean or not*/
int allClean(zBuffer z){
    for(int i = 0;i<z.size;i++)
        if(z.openBuffers[i].dirty) return 0;
    return 1 && E.dirty == 0 ? 1: 0;
}
/* gives length of char* */
int charptrLen(char *str){
    if(str == NULL) return 0;
    else return strlen(str);
}

/* gives name of char* */
char *charptrName(char *str){
    if(str == NULL) return "Untitled";
    else return str;
}

// cy is line whose count of spaces will be used, cx is cursor postion in cy, *l will hold the length of buffer created and *newCx will hold the new location of cursor.
// Used to prependSpaces in a newLine generated
char *prependSpaces(int cy,int cx,int *l,int *newCx){
    erow *row = &E.row[cy];
    int leadingSpaces = 0;
    int i = 0;
    while(i<row->rsize){
        if(row->render[i] == ' ')
            leadingSpaces++;
        else break;
        i++;
    }
    int len = row->size - cx + leadingSpaces;
    leadingSpaces = leadingSpaces <= cx ? leadingSpaces : cx;

    char *buf = malloc(len);
    memset(buf,' ',leadingSpaces);
    snprintf(&buf[leadingSpaces],len-leadingSpaces + 1,&row->chars[cx]);
    buf[len] = '\0';
    
    *l = len;
    *newCx = leadingSpaces;
    return buf;
}
