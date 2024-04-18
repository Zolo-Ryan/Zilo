#include<zBuffer.h>
#include<init.h>
#include<output.h>
#include<fileio.h>
#include<string.h>

/* switch current file buffer */
void switchBuffer(int val){
    if(z.size <= 0) return;
    z.openBuffers[z.currentPointer] = E; // save current buffer state
    
    if(val > 0) z.currentPointer = (z.currentPointer+1)%z.size;
    if(val < 0) z.currentPointer = (z.currentPointer-1+z.size)%z.size;//incorrect math
    
    E = z.openBuffers[z.currentPointer]; // load the buffer
    editorSetStatusMessage("Switched to: %s",E.filename);
}

/* initializes the buffer */
void initZBuffer(int n,char **args){
    if(n <= 1) return;
    z.currentPointer = n-2; // last file
    z.size = n-1;
    z.openBuffers = malloc(n*sizeof(struct editorConfig));
    
    for(int i = 1;i<n;i++){
        initEditor();
        editorOpen(args[i]);
        z.openBuffers[i-1] = E;
    }
}

/* opens a file */
void addBuffer(char *filename){
    int current = z.currentPointer;
    z.openBuffers[current] = E; // saving current state of buffer
    z.openBuffers = realloc(z.openBuffers,(z.size+1)*sizeof(struct editorConfig));
    memmove(&z.openBuffers[current+1],&z.openBuffers[current],(z.size - current)*sizeof(struct editorConfig));
    
    z.size++;

    initEditor();
    if(filename != NULL){ // handling new file opened
        if(access(filename,F_OK) != -1)
            editorOpen(filename);
        else{
            editorSetStatusMessage("Creating a new file: %s",filename);
            E.filename = malloc(strlen(filename)+1);
            memcpy(E.filename,filename,strlen(filename)+1);
        }
    }
        
    z.openBuffers[z.currentPointer] = E; // saving into buffers
}

/* closes a file */
void removeBuffer(){
    if(z.size <= 1){
        free(z.openBuffers);
        z.size--;
        return;
    }
    int current = z.currentPointer;
    memmove(&z.openBuffers[current],&z.openBuffers[current+1],(z.size - current - 1)*sizeof(struct editorConfig));
    z.openBuffers = realloc(z.openBuffers,(z.size-1)*sizeof(struct editorConfig));
    z.size--;
    z.currentPointer = current >= z.size ? z.size - 1 : current;
    E = z.openBuffers[z.currentPointer];
}
