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
- **void editorRefreshScreen**(void)
    - Responsible to draw rows (`editorDrawRows`), menubar(`editorDrawMenuBar`), status bar(`editorDrawStatusBar`) and message bar(`editorDrawMessageBar`) by calling the respective functions. As well as to clear the screen.
- **void editorDrawRows**(struct abuf*)
    - Responsible to render the file on the screen. Uses `editorDrawSidebar` to represent the line number.
    - Takes one parameter `ab` - the buffer into which all the contents will be added and then using write() system call all the contents are written in one go on terminal.
- **void editorScroll**(void)
    - Responsible to handle cursor is located correctly on the editor by setting bounds on it.
    - Handles `coloff` (column offset) and `rowoff` (row offset)
- **void editorDrawStatusBar**(struct abuf*)
    - Responsible to draw the status bar at the last second line of the screen.
    - Takes one parameter `ab` - the buffer into which all the contents will be added and then using write() system call all the contents are written in one go on terminal.
- **void editorDrawMenuBar**(struct abuf*)
    - Responsible to draw menubar at top of editor to display the currently opened files
    - Takes one parameter `ab` - the buffer into which all the contents will be added and then using write() system call all the contents are written in one go on terminal.
- **void editorSetStatusMessage**(const char*,...)
    - Used to set the status message of the editor, which is used by `editorDrawMessageBar` to display the message at the bottom of the editor.
    - This is a variadic function just like `printf`. Specially created to set `E.statusmsg` and `E.statusmsg_time`
- **void editorDrawMessageBar**(struct abuf*)
    - Used to write `E.statusmsg` at the bottom of the screen.
    - Takes one parameter `ab` - the buffer into which all the contents will be added and then using write() system call all the contents are written in one go on terminal.
- **void editorDrawSidebar**(struct abuf*,int)
    - Used to draw the sidebar of the editor.
    - Takes one parameter `struct abuf *ab` - the buffer into which all the contents will be added and then using write() system call all the contents are written in one go on terminal.
    - Second parameter is `filerow`. Used to know which line number is this.

#### 11. row_operations.h
- Header file to perform operations on the row. `editor_operations.h` will use some of `row_operations.h` functions internally
- **void editorInsertRow**(int,char*,size_t)
    - Used to insert a new row into the editor, when enter key is pressed.
    - Takes 3 parameters, first is `at`, the index at which new row is to be inserted.
    - second parameter is `s` - string that is to be put in that row
    - third parameter is `len` - lenght of string used
- **void editorUpdateRow**(erow*)
    - Used to handle the proper rendering of the `\t` - tab character.
    - Takes `row` as parameter - the row which is to be updated.
- **int editorRowCxToRx**(erow*,int)
    - Used to conver `E.cx` of a `row` into the corresponding `E.rx`
    - Takes two parameters `row` - the row whose `rx` is needed
    - second parameter is `cx` the value to be converted to `rx`
    - Returns the corresponding `rx`
- **void editorRowInsertChar**(erow*,int,int)
    - Used to insert a character into the specified editor row.
    - Takes 3 parameters. First parameter is  `row` - The pointer to row into which character is to be inserted.
    - Second parameter is `E.cx` the index in `row` where the character is to be inserted.
    - Third parameter is `c` - character to be inserted.
- **void editorRowDelChar**(erow*,int)
    - Used to delete a character from a row.
    - Needs 2 parameters. First is `row` - pointer to the row whose character is to be deleted.
    - Second is `at` - index in `row` where the character is to be deleted.
- **void editorFreeRow**(erow*)
    - Used to free a `row` created.
    - Takes one parameter `row` - pointer to row which is to be freed.
- **void editorDelRow**(int)
    - Used to delete a whole row
    - Takes one parameter `at` - the row number which is to be deleted.
- **void editorRowAppendString**(erow*,char*,size_t)
    - Used to append a string into a row.
    - Takes 3 parameters `row` - pointer to row
    - Second is `s` - string to be appended.
    - Third `len` - len of string
- **int editorRowRxToCx**(erow*,int)
    - Converts the `E.rx` to corresponding `E.cx` value.
    - `erow *row` - pointer to the row
    - `int rx` - rx to be converted
    - returns the corresponding `cx`

#### 12. terminal.h
- Header file containing all the functions related to interact with the terminal
- **void enableRawMode**(void)
    - Used to disable echo, cooked, etc. modes of the terminal as well as interrupts. This converts the terminal into the form suitable to make our own text editor.
- **void disableRawMode**(void)
    - After exiting the editor, the terminal should be reverted back to its original state. This function handles that.
- **void die**(const char*)
    - A function made for error handling (might move to `utils.h` in next versions)
    - Takes a string `s` as parameter to display the type of error occured.
- **int editorReadKey**(void)
    - Used to read a key pressed by the user. Since some key presses genrate more than one character. This functions handles that and returns the corresponding value for the key pressed.
- **int getWindowSize**(int*,int*)
    - Used to get the dimensions of the terminal. 
    - Returns 0 in case of success, else otherwise
    - Takes two parameters `rows` - pointer used to store the row dimensions of the terminal.
    - `cols` - to store the col dimensions of the terminal
- **int getCursorPosition**(int*,int*)
    - Used to get the current cursor position.
    - Used by `getWindowSize` internally.
    - Returns 0 in case of success, else otherwise

#### 13. utils.h
- A utility header containing all the extra funcions.
- **int is_seperator**(int)
    - Used to check if the given characater belongs to `,.()+-/*=~%<>[];` or ` ` or `\0`.
    - Takes `c` - the character to be tested
    - Returns true if matched, else otherwise.
- **int is_open_character**(int)
    - Used to check if the given character is belongs to `({['"` or not.
    - Takes `c` - the character to be tested
    - Returns the corresponding closing character if matched else -1.
- **int digits**(int)
    - Used to count the number of digits in the given number.
    - Takes `a` - number whose digits is to be counted.
    - Returns the count of digits
- **int allClean**(zBuffer)
    - Used to check if any of the open files are dirty or not.
    - Take `z` - the buffer holding the current opened files
    - Return true if none of the are dirty else false
- **int charptrLen**(char*)
    - Used to return the len of a string.
    - Takes `str` - pointer to string
    - Returns 0 if `str` is NULL else `strlen(str)`
- **char * charptrName**(char*)
    - Used to return the name of file opened.
    - Takes `str` - filename
    - Returns `Untitled` if `filename` is NULL else return `str`

#### 14. zBuffer.h
