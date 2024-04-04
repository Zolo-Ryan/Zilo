#include<main.h>
#include<fileio.h>
#include<terminal.h>
#include<output.h>
#include<input.h>
#include<init.h>
#include<clipboard.h>

int main(int argc, char *argv[])
{
    enableRawMode();
    initEditor();
    if(argc >= 2){
        editorOpen(argv[1]);
    }

    editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find | Ctrl-G = Goto");

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
