# My Shell

- A user defined interactive POSIX based shell program using cpp which supports a semi-colon separated list of commands.

## Features
1. Display: On running the shell program, the prompt of the form '_username_@_system_name_:_current_directory_>' is displayed. The directory from where shell is invoked is considered as home directory and indicated by ~
2. Handling of random spaces and tabs in user input. Also multiple commands can be entered using ';' as a separator.
3. Includes implementation of basic commands such as cd, pwd, echo, ls (with flags) from scratch using system calls.
4. System commands: Foreground and Background processes. Any command ending with '&' is treated as a background command.
5. custom pinfo command that display's process information using process id. 'pinfo': prints info of shell program. 'pinfo pid' prints info of process with pid.
6. Search functionality that searches for the file or directory under current directory recursively and returns true or false.
7. I/O redirection: Input and output redirection using < and > respectively. Example: for output redirection: 'echo "hello" > output.txt' and for input redirection:'cat < example.txt'
8. Pipe: Supports single pipe commands. Use '|' as linking character between two commands. No redirection operators supported.
9. Auto-Suggest and Completion: Uses a Trie data structure to provide auto-suggestions for commands as the user types. When a user presses the Tab key, the shell suggests possible completions based on the input.
10. Simple signals such as CTRL-C (interrupt any currently running foreground process), CTRL-D (logout of shell) and CTRL-Z (pushes any currently running foreground process into the background, and change its state from running to stopped).
11. History: 'history' command by default displays last 10 used commands. 'history num' commands displays only last num commands upto limit of 20. The up and down arrow keys allow users to navigate through their command history.

## Files Overview:
- cd.cpp: contains code which implements cd functionality using chdir system call
- pwd.cpp: implements pwd functionality using getcwd system call
- ls.cpp: implements ls functionality with flags. Mainly uses struct stat and dirent.
- search.cpp: implements recursive search functionality for a given file/directory under current directory. Returns True if such file/dir is found else returns false. uses functions such as opendir, readdir and struct stat
- signal.cpp: implements basic signals such as CTRLZ and CTRLC using signal handlers.
- history.cpp: implements history command functionality. It saves history in a history.text file and loads previous session history when a new session is started. 
- trie.h: Defines the Trie data structure used for command auto-completion. It provides methods to insert commands, search for suggestions based on user input, and initialize the Trie with commands from system directories.
- main.cpp: includes main function and some other functions for display, tokenize input, echo etc.
- headers.h: includes all used headers and function definitions
- makefile: makefile to compile all code.


## How To Run Code
- Navigate to the directory where all code files and makefile is located and open in terminal.
  ```sh
  cd POSIX-Shell
  ```
- run 'make' command in terminal which will create object files for all code files and shell executable.
  ```sh
  make
  ```
- run './shell' command in terminal to start shell program.
  ```sh
  ./shell
  ```

