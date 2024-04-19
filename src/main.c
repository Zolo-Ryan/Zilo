#include<main.h>
#include<fileio.h>
#include<terminal.h>
#include<output.h>
#include<input.h>
#include<init.h>
#include<clipboard.h>
#include<zBuffer.h>

int main(int argc, char *argv[])
{
    enableRawMode();
    if(argc >= 2){
        initZBuffer(argc,argv);
        E = z.openBuffers[z.currentPointer];
    }
    else{
        initEditor();
        z.size++;
        z.currentPointer++;
        z.openBuffers = realloc(z.openBuffers,z.size*sizeof(struct editorConfig));
        z.openBuffers[z.currentPointer] = E;
    }

    editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find | Ctrl-G = Goto");
    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
