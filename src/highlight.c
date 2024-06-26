#include<init.h>
#include<highlight.h>
#include<utils.h>

void editorSelectSyntaxHighlight(){
    E.syntax = NULL;
    if(E.filename == NULL) return;

    char *ext = strrchr(E.filename,'.'); // searches for last occurence of character c unlike strchr which searches for first occurence

    for(unsigned int j = 0;j < HLDB_ENTRIES;j++){
        struct editorSyntax *s = &HLDB[j];
        unsigned int i = 0;
        while(s->filematch[i]){
            int is_ext = (s->filematch[i][0] == '.');
            if((is_ext && ext && !strcmp(ext,s->filematch[i])) || (!is_ext && strstr(E.filename,s->filematch[i]))){
                E.syntax = s;

                int filerow; // highlight file after a file is saved and give a syntax
                for(filerow = 0;filerow < E.numrows;filerow++){
                    editorUpdateSyntax(&E.row[filerow]);
                }
                return;
            }
            i++;
        }
    }
}
int editorSyntaxToColor(int hl){
    switch(hl){
        case HL_MLCOMMENT:
        case HL_COMMENT: return 36; // cyan
        case HL_KEYWORD1: return 33; // yellow
        case HL_KEYWORD2: return 32; // green
        case HL_STRING: return 35; // magenta
        case HL_NUMBER: return 31; // foreground red
        case HL_MATCH: return 34; // blue
        case HL_SIDEBAR: return 40; // magenta back
        default: return 37; // foreground white
    }
}
void editorUpdateSyntax(erow *row){
    row->hl = realloc(row->hl,row->rsize); // realloc since row might be a new row or a longer row
    memset(row->hl,HL_NORMAL,row->rsize); // set hl to normal

    if(E.syntax == NULL) return;

    char **keywords = E.syntax->keywords;

    char *scs = E.syntax->singleline_comment_start;
    char *mcs = E.syntax->multiline_comment_start;
    char *mce = E.syntax->multiline_comment_end;

    int scs_len = scs ? strlen(scs) : 0;
    int mcs_len = mcs ? strlen(mcs) : 0;
    int mce_len = mce ? strlen(mce) : 0;

    int prev_sep = 1; // is prev char a seperator?
    int in_string = 0; // stores either " or ' and used as boolean as well, hence an int
    int in_comment = (row->idx > 0 && E.row[row->idx-1].hl_open_comment); // for multiline comments not single line comment

    int i = 0;
    // order matters in this while loop
    while(i < row->rsize){
        char c = row->render[i];
        unsigned char prev_hl = (i > 0)? row->hl[i-1]: HL_NORMAL;
        
        // single line comment
        if(scs_len && !in_string && !in_comment){
            if(!strncmp(&row->render[i],scs,scs_len)){
                memset(&row->hl[i],HL_COMMENT,row->rsize-i);
                break;
            }
        }

        if(mcs_len && mce_len && !in_string){
            if(in_comment){
                row->hl[i] = HL_MLCOMMENT;
                if(!strncmp(&row->render[i],mce,mce_len)){
                    memset(&row->hl[i],HL_MLCOMMENT,mce_len);
                    i += mce_len;
                    in_comment = 0;
                    prev_sep = 1;
                    continue;
                }else{
                    i++;
                    continue;
                }
            }else if(!strncmp(&row->render[i],mcs,mcs_len)){
                memset(&row->hl[i],HL_MLCOMMENT,mcs_len);
                i += mcs_len;
                in_comment = 1;
                continue;
            }
        }

        // string
        if(E.syntax->flags & HL_HIGHLIGHT_STRINGS){
            if(in_string){
                row->hl[i] = HL_STRING;
                if(c == '\\' && i + 1 < row->rsize){
                    row->hl[i+1] = HL_STRING;
                    i += 2;
                    continue;
                }

                if(c == in_string) in_string = 0;
                i++;
                prev_sep = 1;
                continue;
            }else{
                if(c == '\"' || c == '\''){
                    in_string = c;
                    row->hl[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }

        // numbers
        if(E.syntax->flags & HL_HIGHLIGHT_NUMBERS){
            if((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) || (c == '.' && prev_hl == HL_NUMBER)){
                row->hl[i] = HL_NUMBER;
                i++;
                prev_sep = 0;
                continue;
            }
        }
        
        // keywords
        if(prev_sep){
            int j;
            for(j = 0;keywords[j];j++){
                int klen = strlen(keywords[j]);
                int kw2 = keywords[j][klen-1] == '|';
                if(kw2) klen--;

                if(i+klen < row->rsize && !strncmp(&row->render[i],keywords[j],klen) && is_seperator(row->render[i+klen])){
                    memset(&row->hl[i],kw2 ? HL_KEYWORD2: HL_KEYWORD1,klen);
                    i += klen;
                    break;
                }
            }
            if(keywords[j] != NULL){
                prev_sep = 0; // prev was a keyword
                continue;
            }
        }

        prev_sep = is_seperator(c);
        i++;
    }
    int changed = (row->hl_open_comment != in_comment);
    row->hl_open_comment = in_comment;
    if(changed && row->idx + 1 < E.numrows){
        editorUpdateSyntax(&E.row[row->idx + 1]);
    }
}
