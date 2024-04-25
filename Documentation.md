# Documentation for ZILO

> Before starting I would like to share the refrence from which I created Zilo, [Kilo Text Editor](https://viewsourcecode.org/snaptoken/kilo/)

The source code is divided into 2 directories
- include/
- src/

## include/

`include/` contains all the header files used in zilo
#### 1. buffer.h
- Header file containing all the function declarations related to create a buffer. This buffer will store all the file contents and escape sequences. Then write all the content in one write() call instead of calling write() multiple times
- **struct abuf, ABUF_INIT**
    - they declare the structure and intial state of buffer
- **void abAppend**(struct abuf*,char*,int)
    - function to append a string into the buffer. First parameter is a pointer to buffer, second is the string to be appended and last is the size of string (NULL character excluded);
- **void abFree**(struct abuf*)
    - free the buffer size

#### 2. clipboard.h
- Header file containing all the function declarations and data structure related to create a clipboard. Which will be used for copy paste operation. Currently only whole lines can be copied, cut and pasted. Currently it can only store one string.
- **struct clip, CLIP_INIT**
    - they declare the data structure and initial state of clipboard
- **extern clip clipboard**
    - Creates a clipboard
- **copyToClipboard**(void)
    - Used to copy the line where cursor is currently present. The current line won't be deleted.
- **cutToClipboard**(void)
    - Used to cut the line where cursor is currently present. The current line will be deleted and saved to clipboard.
- **pasteFromClipboard**(void)
    - Used to paste line from clipboard. After pasting, the clipboard will still contain the last copied/cut string.

#### 3. editor_operations.h
- Header file containing function declarations related to insert and delete characters from editor, as well as function for going to a specific line. (Functions specified here are related to editor and not to Row, so don't confuse them with row_operations functions)
- **editorInsertChar**(int)
    - Used to insert a new character into the editor.
    - Internally uses `editorRowInsertChar` (row_operations) to insert the inputted char at the current row.
    - first parameter is used to take the character that is pressed and has to be inserted
- **editorDelChar**(void)
    - Used to delete a character left of the current cursor position. Used for `backspace` and `del` key as well. In case of `del` cursor is moved right and then this function is called, so the character right of cursor is deleted
    - Internally uses `editorRowDelChar` (row_operations) to delete the char from current row
- **editorInsertNewLine**(void)
    - Used to insert a new line into the editor whenever `Enter` is pressed.
    - Uses `editorInsertRow` (row_operations) internally
- **goToLine**(void)
    - A wrapping function for `goToLineCallback` - which actually moves cursor to the specifie line.
    - Internally used `editorPrompt` (input) to append the character into the query string of `goToLineCallback`
    - Used to free the query inputted as well as restore the cursor back to its positon in case of cancel (`esc`)
- **goToLineCallback**(char*,int)
    - first parameter is the `query` string which user will input. Using this string we will place the cursor at the requested position
    - second parameter is `key` which will append into `query` as the user presses any key.

#### 4. fileio.h
- Header file containing function declarations on how to open and save a file and convert the editor rows into string for saving.
- **void editorOpen**(char*)
    - To open the specified file
    - first parameter is `filename` to be opened.
- **char * editorRowsToString**(int*)
    - To convert the editor rows into a single string. That is all the content written on editor will be saved into a buffer. This buffer will then be written into the file.
    - Returns the pointer to buffer storing the file.
    - First parameter is used to store the length of buffer created, so that the calling function know the size of buffer.
- **void editorSave**(void)
    - To save the contents of buffer created in `editorRowsToString` into the file. While checking if a new file was created or not.
    - This function also manages the save abort by the user.

#### 5. finder.h
- Header file containing all the function declarations used to find a word in the editor. The cursor will be moved to the starting character of that word. Color of the word will also be changed. (Bug: currently after the search operation is completed the color of the last found word is not reverted back to normal)
- **void editorFind**(void)
    - A wrapper function for `editorFindCallback`.
    - Used to save the current cursor position, to be used when user cancel (`esc`) the operation.
    - Frees the `query` string created by the `editorFindCallback` function.
- **void editorFindCallback**(char*,int)
    - Used to actually find the `query` string inputted by the user into the `editorPrompt` in the editor.
    - Used to cycle through all the `query` strings matched in the editor

#### 6. highlight.h
- Header file containing all the function declarations used to highlight the syntax. (currently only C is supported. However more languages are welcomed to be added)
- **void editorUpdateSyntax**(erow*)
    - Updates the highlighting of a specified editor row.
    - Takes one pointer to row whose highlighting is to be updated
- **int editorSyntaxToColor**(int)
    - Converts the Higlight Magic Numbers to color numbers
    - Takes one parameter that is the highlight magic number
- **void editorSelectSyntaxHighlight**(void)
    - Selects the syntax styling to be used for a specific file extension by searching for it in the database

#### 7. init.h
- Header file containing functions related to initialization of editor
- **void initEditor**(void)
    - Fills the current editor variables with a default value.
    - Also responsible to get the window size for editor
- Also initializes some other constants like `zBuffer z`, `C_HL_extensions`, `C_HL_keywords` and `HLDB`.

#### 8. input.h
- Header file related to passing inputs into the editor.
- **void editorProcessKeypress**(void)
    - Waits for a keypress and handles it
    - Internally uses `editorReadKey` (terminal) to get the keypress
- **void editorMoveCursor**(int)
    - Used to move cursor in different directions
    - Takes one parameter `key` which is used to identify which arrow key is pressed
- **char * editorPrompt**(char*,void(* callback)(char *,int))
    - Used to display a one line prompt at the bottom of the screen. This is used to take in user input while performing search, goto, save etc. operations.
    - Returns the `query` string formed in the prompt.
    - Takes two parameters `prompt` - The string to be displayed to user so as to tell what type of input is required. `callback` - function to be called for partial query passed by the user. Useful for incremental search feature. where user don't have to type whole string to find a match, instead he will see a match for each character he types.

#### 9. main.h
- Header file related to various data structures and enum definitions, along with the magic numbers
- **enum editorKey**
    - contains constants to represent non-printable keypresses
- **enum editorHighlight**
    - contains constants for different highlighting syntax
- **struct editorSyntax**
    - contains the information about the highlighting rule to follow for a specific file extension
- **typedef struct erow**
    - creates a new datatype for editor row. containing index of row in file, size of row, actual value for row, rendered value of row, highlighting to be used, etc.
- **struct editorConfig**
    - creates a data structure to store information regarding the editors present state, like cursor position, row and col offset, no of rows and cols, sidebar_width, filename, status message etc.

#### 10. output.h
- Header file to handle all the output (visible to user) operations.
- **editorRefreshScreen**(void)
    - ok
- **editorDrawRows**(struct abuf*)
    - ok
- **editorScroll**(void)
    - ok
- **editorDrawStatusBar**(struct abuf*)
    - ok
- **editorDrawMenuBar**(struct abuf*)
    - ok
- **editorSetStatusMessage**(const char*,...)
    - ok
- **editorDrawMessageBar**(struct abuf*)
    - ok
- **editorDrawSidebar**(struct abuf*,int)
    - ok