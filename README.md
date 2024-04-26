# A text editor in C
Made from scratch <br>
ref: [Kilo Text Editor](https://viewsourcecode.org/snaptoken/kilo/index.html)

Documentation: [Here](./Documentation.md)

## Additional functionalities

- [x] Open and close brackets, quotes
- [x] Goto Line
- [x] Open multiple files
- [x] Menubar to display open files
- [x] Sidebar to display line numbers
- [x] Close a specific file
- [x] Cut, Copy and paste whole lines
- [ ] Cut, Copy and paste selected area
- [ ] Relative line numbers
- [ ] Syntax Higlighting for java
- [ ] Auto indent
- [ ] Scrolling with touchpad/mouse
- [ ] Debian package created

## Setup
- Clone the repository
- Open terminal in the root directory of the repository
- Run `make`
- `bin/kilo` is the program created
- run `bin/./kilo` without any arguments else give it a file name present in current directory

## Installation
zilo.deb is package build using bin/kilo. If you want you can install zilo on your linux system as well. Just run the following command:
```
sudo apt install ./zilo.deb
```

## Shortcuts

- `Ctrl-s` => save current file
- `Ctrl-o` => open a file
- `Ctrl-n` => open a new file
- `Ctrl-g` => go to a line
- `Ctrl-w` => close current file
- `Ctrl-q` => quit the editor
- `Ctrl-f` => find a word in current file
- `Ctrl-c` => copy current line
- `Ctrl-v` => paste line in clipboard
- `Ctrl-x` => cut current line
- `Ctrl-t` => view next file from menubar
- `Ctrl-r` => view prev file from menubar
- `Home` => To reach the starting of current line
- `End` => To reach the end of current line
- `Page Up` => To move one page up
- `Page Down` => To move one page down

### Description
<p>I created this text editor by taking refrence from kilo text editor. This is made purely for educational purposes. I had fun creating this masterpiece. Eventhough it is not anywhere near the modern text editor but the fact I created it of my own is important. You can open, save, close any file you want in this editor. You can search for a word, goto a specific line, switch between open files along with copy, cut and paste whole lines. A menubar to display the open files and the current file in focus.</p>
<p>I learned a lot of memory management as well as new libraries and their amazing features in this mini project.</p>
<p>Their is a good chance I won't be completing all the additional functionalities I have added above.</p>

### Bugs

Some bugs I am too lazy to fix.

- Menubar give up when there are too many files or the file name is too large
- After a find search is done, the color is not reverted back to normal

