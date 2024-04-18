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
