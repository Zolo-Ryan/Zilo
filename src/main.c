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
    if(argc == 2){
        if(!strcmp(argv[1],"-h")){
            fprintf(stdout,"Type man zilo for help\n");
            exit(EXIT_SUCCESS);
        }else if(!strcmp(argv[1],"-v")){
            fprintf(stdout,"Zilo version is: %s\n",KILO_VERSION);
            exit(EXIT_SUCCESS);
        }else if(!strcmp(argv[1],"-hv") || !strcmp(argv[1],"-vh")){
            fprintf(stdout,"%s\n",KILO_VERSION);
            fprintf(stdout,"Type man zilo for help\n");
            exit(EXIT_SUCCESS);
        }else if(argv[1][0] == '-'){
            fprintf(stdout,"Type man zilo for help");
            exit(EXIT_SUCCESS);
        }
    }
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
        if(getWindowSize(&E.screenrows,&E.screencols) == -1) die("getWindowSize");
        E.screenrows -= 3;
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
