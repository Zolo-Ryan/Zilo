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
