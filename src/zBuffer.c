#include<zBuffer.h>
#include<init.h>
#include<output.h>
#include<fileio.h>

void switchBuffer(int val){
    if(val > 0) z.currentPointer = (z.currentPointer+1)%z.size;
    if(val < 0) z.currentPointer = (z.currentPointer-1+z.size)%z.size;//incorrect math
    E = z.openBuffers[z.currentPointer];
    editorSetStatusMessage("Switched to: %s",E.filename);
}
void initZBuffer(int n,char **args){
    z.currentPointer = 0;
    z.size = n-1;
    z.openBuffers = malloc(n*sizeof(struct editorConfig));
    
    for(int i = 1;i<n;i++){
        initEditor();
        editorOpen(args[i]);
        z.openBuffers[i-1] = E;
    }
}