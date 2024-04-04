#include<fileio.h>
#include<input.h>
#include<output.h>
#include<highlight.h>
#include<row_operations.h>
#include<terminal.h>

void editorSave(void){
    if(E.dirty <= 0){
        editorSetStatusMessage("Nothing to save");
        return;
    }
    if(E.filename == NULL){
        E.filename = editorPrompt("Save as: %s",NULL);
        if(E.filename == NULL){
            editorSetStatusMessage("Save aborted");
            return;
        }
        editorSelectSyntaxHighlight();
    }

    int len;
    char *buf = editorRowsToString(&len);
    int fd = open(E.filename,O_RDWR | O_CREAT, 0644); // O_TRUNC truncates the file completely, making it an empty file
    if(fd != -1){
        if(ftruncate(fd,len) != -1){
            if(write(fd,buf,len) == len){
                close(fd);
                free(buf);
                editorSetStatusMessage("%d bytes written to disk",len);
                E.dirty = 0; // after saving file is clean
                return;
            }
        }
        close(fd);
    }
    free(buf);
    editorSetStatusMessage("Can't Save! I/O error: %s",strerror(errno));
    // advanced editors will write to a new temporary file and then rename that file to the actual file the user wants to overwrite
}
char *editorRowsToString(int *buflen){
    int totlen = 0;
    int j;
    for(j = 0;j<E.numrows;j++)
        totlen += E.row[j].size + 1;
    *buflen = totlen;
    char *buf = malloc(totlen);
    char *p = buf;
    for(j = 0;j<E.numrows;j++){
        memcpy(p,E.row[j].chars,E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}
void editorOpen(char *filename){
    free(E.filename);
    E.filename = strdup(filename); // makes a copy of string, allocating req memory and assuming you will free that memory

    editorSelectSyntaxHighlight();

    FILE *fp = fopen(filename,"r");
    if(!fp) die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    // buffer, buffer len, ptr. buffer is allocated memory automatically and len is updated as well
    while((linelen = getline(&line,&linecap,fp)) != -1){ // -1 at EOF
        while(linelen > 0 && (line[linelen-1] == '\n' || line[linelen-1] == '\r'))
            linelen--; // truncating \r\n from back of string
        editorInsertRow(E.numrows,line,linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
}
