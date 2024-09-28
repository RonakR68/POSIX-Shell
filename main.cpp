#include "headers.h"
#include "trie.h"
using namespace std;

// Function to enable raw mode
void enableRawMode(struct termios &orig_termios)
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);   // Get current terminal settings
    raw = orig_termios;                       // Copy settings to raw
    raw.c_lflag &= ~(ICANON | ECHO);          // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); // Apply the new settings
}

// Function to disable raw mode
void disableRawMode(struct termios &orig_termios)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); // Restore original settings
}

string read_input(Trie &commandTrie, vector<string> &history, size_t &currentHistoryIndex)
{
    char c[100000]; // Buffer for input
    int i = 0;      // Index for buffer
    vector<string> suggestionsList;
    struct termios orig_termios;
    enableRawMode(orig_termios);
    

    while (true)
    {
        char ch = getchar(); // Read a character from input
        
        if (ch == '\n')
        { // Check for Enter key
            if (i > 0)
            {
                cout << endl;
                break; // Exit loop if there's input
            }
        }
        else if (ch == 0x7f)
        { // Check for Backspace
            if (i > 0)
            {
                cout << "\b \b"; // Move cursor back, print space, move back again
                i--;             // Decrease index
            }
        }
        else if (ch == '\t')
        { 
            // Check for Tab key
            c[i] = '\0';
            string input(c);
            stringstream ss(input);
            string lastWord;
            while (ss >> lastWord);
            int lastWordLength = lastWord.length();
            suggestionsList = commandTrie.autocomplete(lastWord); // Get suggestions
            if (!suggestionsList.empty()){
                string firstSuggestion = suggestionsList[0];
                // If there's only one suggestion, auto-complete
                if (suggestionsList.size() == 1){
                    // Clear the current last word from the terminal
                    for (int j = 0; j < lastWordLength; j++)
                    {
                        cout << "\b \b"; // Move back and erase each character
                    }
                    i -= lastWordLength; // Reset index for the last word

                    // Insert the auto-completed suggestion
                    for (char ch : firstSuggestion)
                    {
                        c[i++] = ch;
                        cout << ch; // Print the suggestion inline
                    }
                }
                else
                {
                    // If there are multiple suggestions, show all of them
                    cout << endl;
                    for (size_t j = 0; j < suggestionsList.size(); j++)
                    {
                        cout << suggestionsList[j] << " ";
                    }
                    cout << endl;

                    // After displaying suggestions, restore the input
                    for (int j = 0; j < i; j++)
                    {
                        cout << c[j]; // Reprint characters from the buffer (keeping input intact)
                    }
                }
            }
        }
        else if (ch == 0x1B) { // Check for Escape sequence (arrow keys)
            getchar(); // skip the [
            switch (getchar()) {
                case 'A': // Up arrow
                    if (!history.empty() && currentHistoryIndex > 0) {
                        currentHistoryIndex--;
                        string command = history[currentHistoryIndex];
                        cout << "\r\033[K"; // Clear current line
                        cout << command;    // Show command from history
                        strcpy(c, command.c_str()); // Update buffer
                        i = command.length(); // Update index
                        cout.flush(); // Flush output to show the command
                    }
                    break;
                case 'B': // Down arrow
                    if (!history.empty() && currentHistoryIndex < history.size() - 1) {
                        currentHistoryIndex++;
                        string command = history[currentHistoryIndex];
                        cout << "\r\033[K"; // Clear current line
                        cout << command;    // Show command from history
                        strcpy(c, command.c_str()); // Update buffer
                        i = command.length(); // Update index
                        cout.flush(); // Flush output to show the command
                    } else if (currentHistoryIndex == history.size() - 1) {
                        // If at the end of the history, clear the line
                        currentHistoryIndex++;
                        cout << "\r\033[K"; // Clear current line
                        i = 0; // Reset index
                        c[0] = '\0'; // Clear buffer
                    }
                    break;
            }
        }
        else
        {                // Add character to buffer
            c[i++] = ch; // Store character
            cout << ch;  // Echo character to output
        }
    }
    c[i] = '\0'; // Null terminate the string
    disableRawMode(orig_termios);
    return string(c); // Return input as a string
}

// display shell prompt
void displayShell(string user, string host, string dir)
{
    cout << user << "@" << host << ":" << dir << ">";
}

// get multiple commands if any from single line
void getCommands(char *input, vector<string> &commands)
{
    char *command = strtok(input, ";");
    while (command != NULL)
    {
        commands.push_back(command);
        command = strtok(NULL, ";");
    }
}

// tokenize single input command
void Tokenize(char *input, char *argv[])
{
    for (int i = 0; i < 256; i++)
    {
        argv[i] = NULL;
    }
    char *str_Tokenized;
    str_Tokenized = strtok(input, " \t");
    int idx = 0;
    while (str_Tokenized != NULL)
    {
        argv[idx++] = str_Tokenized;
        str_Tokenized = strtok(NULL, " ");
    }
    argv[idx] = NULL;
}

bool isInt(string str)
{
    if (str.empty())
    {
        return false;
    }
    for (char c : str)
    {
        if (!isdigit(c))
        {
            return false;
        }
    }
    return true;
}

void parse_redir(char *argv[], int n, vector<string> &redir_argv)
{
    int idx = 0;
    while (argv[idx] != NULL)
    {
        // Check if command contains character <, >
        if (strcmp(argv[idx], "<") == 0 || strcmp(argv[idx], ">") == 0)
        {
            // Check for succeeded file name
            if (idx + 1 < n)
            {
                // Move redirect type and file name to redirect arguments vector
                redir_argv[0] = strdup(argv[idx]);
                redir_argv[1] = strdup(argv[idx + 1]);
                argv[idx] = NULL;
                argv[idx + 1] = NULL;
            }
        }
        idx++;
    }
}

int parse_pipe(char *argv[], char *child01_argv[], char *child02_argv[])
{
    int idx = 0, split_idx = 0;
    bool contains_pipe = false;
    int cnt = 0;

    while (argv[idx] != NULL)
    {
        // Check if user_command contains pipe character |
        if (strcmp(argv[idx], "|") == 0)
        {
            split_idx = idx;
            contains_pipe = true;
        }
        idx++;
    }

    if (!contains_pipe)
    {
        return 0;
    }

    // Copy arguments before split pipe position to child01_argv[]
    for (idx = 0; idx < split_idx; idx++)
    {
        child01_argv[idx] = strdup(argv[idx]);
    }
    child01_argv[idx++] = NULL;

    // Copy arguments after split pipe position to child02_argv[]
    while (argv[idx] != NULL)
    {
        child02_argv[idx - split_idx - 1] = strdup(argv[idx]);
        idx++;
    }
    child02_argv[idx - split_idx - 1] = NULL;
    return 1;
}

void exec_with_pipe(char *child01_argv[], char *child02_argv[])
{
    int pipefd[2];

    if (pipe(pipefd) == -1)
    {
        /* Create a pipe with 1 input and 1 output file descriptor
        Notation: Index = 0 ==> read pipe, Index = 1 ==> write pipe
        */
        perror("pipe() failed");
        exit(EXIT_FAILURE);
    }

    // Create 1st child
    if (fork() == 0)
    {

        // Redirect STDOUT to output part of pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        execvp(child01_argv[0], child01_argv);
        perror("Fail to execute first command");
        exit(EXIT_FAILURE);
    }

    // Create 2nd child
    if (fork() == 0)
    {

        // Redirect STDIN to input part of pipe
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);

        execvp(child02_argv[0], child02_argv);
        perror("Fail to execute second command");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    // Wait for child 1
    wait(0);
    // Wait for child 2
    wait(0);
}

void child(char *argv[], bool background, bool oredir, bool aredir, bool iredir, vector<string> &redir_argv)
{
    int fd_out, fd_in;
    if (oredir || aredir)
    {
        // Redirect output
        // Get file description
        fd_out = creat(redir_argv[1].c_str(), S_IRWXU);
        if (fd_out == -1)
        {
            perror("Redirect output failed");
            exit(EXIT_FAILURE);
        }

        // Replace stdout with output file
        dup2(fd_out, STDOUT_FILENO);

        // Check for error on close
        if (close(fd_out) == -1)
        {
            perror("Closing output failed");
            exit(EXIT_FAILURE);
        }
    }

    else if (iredir)
    {
        // Redirect input
        fd_in = open(redir_argv[1].c_str(), O_RDONLY);
        if (fd_in == -1)
        {
            perror("Redirect input failed");
            exit(EXIT_FAILURE);
        }

        if (dup2(fd_in, STDIN_FILENO) == -1)
        {
            perror("Duplication of input file descriptor failed");
            exit(EXIT_FAILURE);
        }

        if (close(fd_in) == -1)
        {
            perror("Closing input failed");
            exit(EXIT_FAILURE);
        }
    }
    if (execvp(argv[0], argv) == -1)
    {
        perror("Fail to execute command");
        exit(EXIT_FAILURE);
    }
}

void parent(pid_t child_pid, bool isbackground)
{
    int status;
    // printf("Parent <%d> spawned a child <%d>.\n", getpid(), child_pid);

    if (!isbackground)
    {
        waitpid(child_pid, &status, 0);
    }

    else
    {
        waitpid(child_pid, &status, WUNTRACED);
        if (WIFEXITED(status))
        {
            // printf("Child <%d> exited with status = %d.\n", child_pid, status);
        }
    }
}

// handle pinfo
void executePinfo(char *args[], int n)
{
    handlePinfo(args, n);
}

int main()
{
    string input;
    size_t hostname_size = 256;
    char hostname[hostname_size];
    size_t home_path_size = 1024;
    char home_dir_path[home_path_size];
    char *home_dir = get_wd(home_dir_path, home_path_size); // set home dir
    string home = home_dir;
    int home_len = home.size();
    string user = string(getenv("USER"));
    gethostname(hostname, hostname_size);
    char path[1024];
    size_t buf_size = 1024;
    string prev_dir = "";
    signal(SIGTSTP, signalCtrlZ);
    signal(SIGINT, signalCtrlC);
    loadHistory();
    size_t currentHistoryIndex = history.size();

    Trie commandTrie;
    vector<string> commands = {"cd", "ls", "pwd", "echo", "exit", "pinfo", "search", "history"};
    for (const string &command : commands)
    {
        commandTrie.insert(command);
    }

    while (1)
    {
        char *curr_dir = get_wd(path, buf_size); // get current dir
        string curr_path = curr_dir;
        if (strcmp(curr_path.c_str(), home_dir) == 0)
            curr_path = "~";
        else
            curr_path = "~/" + curr_path.substr(home_len + 1);

        // display
        displayShell(user, string(hostname), curr_path);

        // read input line
        //  if(!getline(cin,input)){
        //      //CTRL+D
        //      cout<<endl;
        //      break;
        //  }

        input = read_input(commandTrie, history, currentHistoryIndex);
        if (input.size() == 0)
            continue;
        currentHistoryIndex = history.size(); 
        addHistory(input);

        // tokenize input line for multiple commands
        vector<string> commands;
        getCommands(&input[0], commands);
        int commands_size = commands.size();
        for (int i = 0; i < commands_size; i++)
        {
            char *argv[256];
            bool isBackground = false;

            // Trim leading spaces
            size_t pos = commands[i].find_first_not_of(" \t");
            if (pos == string::npos)
            {
                continue;
            }

            bool oRedir = false;
            bool aRedir = false;
            bool iRedir = false;
            string redirCmd = "";
            int redirInd = -1;
            if (commands[i].find(">>") != string::npos)
            {
                redirCmd = commands[i];
                redirInd = commands[i].find(">>") + 1;
                aRedir = true;
            }
            if (commands[i].find(">") != string::npos)
            {
                oRedir = true;
                redirCmd = commands[i];
                redirInd = commands[i].find(">");
            }
            if (commands[i].find("<") != string::npos)
            {
                redirCmd = commands[i];
                redirInd = commands[i].find("<");
                iRedir = true;
            }

            // echo
            commands[i] = commands[i].substr(pos);
            if (strcmp(commands[i].substr(0, 4).c_str(), "echo") == 0 && !oRedir && !iRedir && !aRedir)
            {
                cout << commands[i].substr(5) << endl;
                continue;
            }

            // Tokenize each command for tabs and spaces
            Tokenize(&commands[i][0], argv);
            if (argv[0] == NULL)
                break;

            // exit
            if (strcmp(argv[0], "exit") == 0)
            {
                saveHistory();
                return 0;
            }

            // pipe
            char *child1[3], *child2[3];
            if (parse_pipe(argv, child1, child2))
            {
                exec_with_pipe(child1, child2);
                continue;
            }

            // background command check
            int index = 0;
            while (argv[index] != NULL)
            {
                index++;
            }
            int ip_size = index;
            if (index > 0)
            {
                char *lastArg = argv[index - 1];
                int len = strlen(lastArg);
                if (len > 0)
                {
                    if (lastArg[len - 1] == '&')
                    {
                        lastArg[len - 1] = '\0';
                        isBackground = true;
                    }
                }
            }

            // pwd
            if (strcmp(argv[0], "pwd") == 0 && !oRedir && !iRedir && !aRedir)
            {
                char *curr_dir;
                curr_dir = get_wd(path, buf_size);
                if (curr_dir)
                    cout << curr_dir << endl;
                continue;
            }

            // cd
            if (strcmp(argv[0], "cd") == 0 && !oRedir && !iRedir && !aRedir)
            {
                if (ip_size == 1)
                {
                    string old = prev_dir;
                    prev_dir = curr_dir;
                    if (!ch_dir(home_dir))
                    {
                        prev_dir = old;
                    }
                    continue;
                }
                if (ip_size > 2)
                {
                    cout << "Invalid Arguments" << endl;
                    break;
                }
                string cpath = get_wd(path, buf_size);
                if (strcmp(argv[1], "..") == 0 && strcmp(cpath.c_str(), home_dir) == 0)
                {
                    prev_dir = curr_dir;
                    cout << cpath;
                }
                else if ((strcmp(argv[1], "~") == 0) || (strcmp(argv[1], "~/") == 0))
                {
                    string old = prev_dir;
                    prev_dir = curr_dir;
                    if (!ch_dir(home_dir))
                    {
                        prev_dir = old;
                    }
                }
                else if ((strcmp(argv[1], "-") == 0))
                {
                    if (prev_dir.size() == 0)
                    {
                        cerr << "cd: OLDPWD not set" << endl;
                    }
                    else
                    {

                        string dirToChange = prev_dir;
                        prev_dir = curr_dir;
                        if (!ch_dir(dirToChange.c_str()))
                        {
                            prev_dir = dirToChange;
                        }
                    }
                }
                else
                {
                    string old = prev_dir;
                    prev_dir = curr_dir;
                    if (!ch_dir(argv[1]))
                    {
                        prev_dir = old;
                    }
                }
                continue;
            }

            // ls
            if (strcmp(argv[0], "ls") == 0 && !oRedir && !iRedir && !aRedir)
            {
                bool showHidden = false;
                bool longList = false;
                vector<string> dirPath;
                for (int i = 1; i < ip_size; i++)
                {
                    if (strcmp(argv[i], "-a") == 0)
                    {
                        showHidden = true;
                    }
                    else if (strcmp(argv[i], "-l") == 0)
                    {
                        longList = true;
                    }
                    else if (strcmp(argv[i], "-la") == 0)
                    {
                        longList = true;
                        showHidden = true;
                    }
                    else if (strcmp(argv[i], "-al") == 0)
                    {
                        longList = true;
                        showHidden = true;
                    }
                    else if (strcmp(argv[i], "~") == 0)
                    {
                        dirPath.push_back(home);
                    }
                    else
                    {
                        dirPath.push_back(argv[i]);
                    }
                }
                if (dirPath.size() == 0)
                {
                    dirPath.push_back(".");
                }
                listDir(dirPath, showHidden, longList);
                continue;
            }

            // pinfo command
            if (strcmp(argv[0], "pinfo") == 0 && !oRedir && !iRedir && !aRedir)
            {
                executePinfo(argv, ip_size);
                continue;
            }

            // search
            if (strcmp(argv[0], "search") == 0 && !oRedir && !iRedir && !aRedir)
            {
                if (ip_size != 2)
                {
                    cout << "Invalid Arguments" << endl;
                    break;
                }
                cout << boolalpha << searchDir(".", argv[1]) << endl;
                continue;
            }

            // history
            if (strcmp(argv[0], "history") == 0 && !oRedir && !iRedir && !aRedir)
            {
                if (ip_size == 1)
                {
                    displayHistory();
                    continue;
                }
                else
                {
                    string n = argv[1];
                    if (isInt(n))
                    {
                        getHistory(stoi(n));
                        break;
                    }
                    else
                    {
                        cout << "Invalid Arguments" << endl;
                        break;
                    }
                }
            }

            else
            {
                vector<string> redir_argv(2, "");
                if (oRedir || iRedir || aRedir)
                {
                    parse_redir(argv, ip_size, redir_argv);
                }
                pid_t pid = fork();
                switch (pid)
                {
                case -1:
                    perror("fork() failed!");
                    exit(EXIT_FAILURE);

                case 0: // In child process
                    child(argv, isBackground, oRedir, aRedir, iRedir, redir_argv);
                    exit(EXIT_SUCCESS);

                default: // In parent process
                    parent(pid, wait);
                }
            }
        }
    }
    saveHistory();
    return 0;
}