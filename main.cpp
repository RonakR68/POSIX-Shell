#include "headers.h"
using namespace std;
    
//display shell prompt
void displayShell(string user, string host, string dir){
    cout<<user<<"@"<<host<<":"<<dir<<">";
}

// get multiple commands if any from single line
void getCommands(char* input, vector<string>&commands){
    char* command = strtok(input,";");
    while(command != NULL){
        commands.push_back(command);
        command = strtok(NULL, ";");
    }
}

//tokenize single input command
void Tokenize(char* input, vector<string>&v){
	char *str_Tokenized;
	str_Tokenized = strtok(input, " \t");
	while(str_Tokenized != NULL)
	{
		v.push_back(str_Tokenized);
		str_Tokenized = strtok(NULL, " ");
	}
}

bool isInt(string str){
    if(str.empty()){
        return false;
    }
    for(char c:str){
        if(!isdigit(c)){
            return false;
        }
    }
    return true;
}

void parse_redir(vector<string> &argv, vector<string> &redir_argv){
    //cout<<"parse redir"<<endl;
    int idx = 0;
    int n = argv.size();
    //cout<<n<<endl;
    while(idx < n){
        //cout<<idx<<" "<<argv[idx]<<endl;
        // Check if command contains character <, >
        if(strcmp(argv[idx].c_str(), "<") == 0 || strcmp(argv[idx].c_str(), ">") == 0){
            // Check for succeeded file name
            //cout<<"inside parse redir, found "<<argv[idx]<<" at index "<<idx<<endl;
            if(idx+1<n){
                // Move redirect type and file name to redirect arguments vector
                redir_argv[0] = strdup(argv[idx].c_str());
                redir_argv[1] = strdup(argv[idx + 1].c_str());
                if(redir_argv[0]=="<"){
                    argv[idx] = redir_argv[0];
                    argv[idx + 1] = redir_argv[1];
                }
                else{
                    argv[idx] = '\0';
                    argv[idx + 1] = '\0';
                }
                    
            }
            else{
                string cmd = argv[idx];
                int ind1 = cmd.find(">");
                int ind2 = cmd.find("<");
                if(ind1 != -1){
                    string file = cmd.substr(ind1);
                    string sym = to_string(cmd[ind1]);
                    redir_argv[0] = strdup(sym.c_str());
                    redir_argv[1] = strdup(file.c_str());
                    argv[idx] = '\0';
                }
                else if(ind2 != -1){
                    string file = cmd.substr(ind2);
                    string sym = to_string(cmd[ind2]);
                    redir_argv[0] = strdup(sym.c_str());
                    redir_argv[1] = strdup(file.c_str());
                    argv[idx] = '\0';
                }
            }
        }
        idx++;
    }
}

//execute other commands
void execCmd(vector<string> &v, bool background, bool oredir, bool aredir, bool iredir, vector<string> &redir_argv){
    pid_t pid = fork();
    if(pid == 0){
        //child process
        int fd_out, fd_in;
        if(oredir || aredir){
            //cout<<"inside oredir"<<endl;
            // Redirect output
            // Get file description
            //cout<<redir_argv[1]<<endl;
            fd_out = creat(redir_argv[1].c_str(), S_IRWXU);
            if(fd_out == -1){
                perror("Redirect output failed");
                exit(EXIT_FAILURE);
            }

            // Replace stdout with output file
            dup2(fd_out, STDOUT_FILENO);

            // Check for error on close
            if (close(fd_out) == -1){
                perror("Closing output failed");
                exit(EXIT_FAILURE);
            }
        }

            
        if(iredir){
            //cout<<"inside iredir"<<endl;
            // Redirect input
            //cout<<redir_argv[1]<<endl;
            fd_in = open(redir_argv[1].c_str(), O_RDONLY);
            if(fd_in == -1){
                perror("Redirect input failed");
                exit(EXIT_FAILURE);
            }

            dup2(fd_in, STDIN_FILENO);

            if(close(fd_in) == -1){
                perror("Closing input failed");
                exit(EXIT_FAILURE);
            }
        }

        int s = v.size();
        char **inputs = new char *[s+1];
        for (int i = 0; i < s; i++){
            inputs[i] = const_cast<char *>(v[i].c_str());
        }
        inputs[s] = nullptr;
        int result = execvp(inputs[0], inputs);
        if(result == -1){
            perror("Error in execution.");
            exit(EXIT_FAILURE);
        }
    } 
    else if(pid > 0){ 
        // Parent process
        if(background){
            cout<<pid<<endl; //print pid
        }
        else{
            foreground = pid;
            int status;
            waitpid(pid, &status, WUNTRACED); //wait for foreground process
            foreground = -1;
        }
    } 
    else{ 
        // Fork error
        cerr<<"Fork failed."<<endl;
    }
}

//handle pinfo
void executePinfo(const vector<string>& args){
    handlePinfo(args);
}

//handle redirection


bool rawMode = false;

// Function to enable raw terminal input (disable line buffering)
void enableRawMode() {
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    
    term.c_lflag &= ~(ECHO | ICANON | ISIG);
    term.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    term.c_cflag |= (CS8);
    term.c_oflag &= ~(OPOST);
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    rawMode = true;
}

// Function to disable raw terminal input (restore original settings)
void disableRawMode() {
    if (!rawMode) {
        return;
    }

    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    term.c_lflag |= (ECHO | ICANON | ISIG);
    
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    rawMode = false;
}

// Function to read a single character from the terminal
char readChar() {
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
}

// Function to handle up arrow key press
void handleUpArrow(const std::vector<std::string>& history, std::string& input, size_t& historyIndex) {
    if (!history.empty() && historyIndex < history.size()) {
        if (historyIndex == 0) {
            // Save the current input before navigating history
            input = input;
        }

        // Display the next command from history
        std::cout << "\033[2K\r"; // Clear the current line
        std::cout << "> " << history[historyIndex];
        input = history[historyIndex];
        historyIndex++;

        // Print the updated shell prompt
        std::cout << "> " << input;
    }
}

int main(){
    //enableRawMode();
    string input;
    size_t hostname_size = 256;
    char hostname [hostname_size];
    size_t home_path_size = 1024;
    char home_dir_path[home_path_size];
    char* home_dir = get_wd(home_dir_path,home_path_size); //set home dir
    string home = home_dir;
    int home_len = home.size();
    string user = string(getenv("USER"));
    gethostname(hostname,hostname_size);
    char path[1024];
    size_t buf_size = 1024;
    string prev_dir = "";
    signal(SIGTSTP, signalCtrlZ);
    signal(SIGINT, signalCtrlC);
    loadHistory();
    size_t currentHistoryIndex = history.size()-1;
    
    while(1){
        char* curr_dir = get_wd(path, buf_size); //get current dir
        string curr_path = curr_dir;
        if(strcmp(curr_path.c_str(), home_dir) == 0)
            curr_path = "~";
        else curr_path = "~/" + curr_path.substr(home_len+1);

        //display 
        displayShell(user, string(hostname), curr_path);

        //read input line
        if(!getline(cin,input)){
            //CTRL+D
            cout<<endl;
            break;
        }
        if(input.size() == 0) continue;
        addHistory(input);

        //Handle up arrow key press
        // char c = readChar();
        // if (c == '\033') { // Escape sequence
        //     char seq[3];
        //     if (read(STDIN_FILENO, &seq[0], 1) == 1 &&
        //         read(STDIN_FILENO, &seq[1], 1) == 1) {
        //         if (seq[0] == '[' && seq[1] == 'A') {
        //             handleUpArrow(history, input, currentHistoryIndex);
        //             continue;
        //         }
        //     }
        // }
        // disableRawMode();

        //tokenize input line for multiple commands
        vector<string> commands;
        getCommands(&input[0], commands);
        int commands_size = commands.size();
        for(int i=0; i<commands_size; i++){
            vector<string> ip;
            bool isBackground = false;

            // Trim leading spaces
            size_t pos = commands[i].find_first_not_of(" \t");
            if (pos == string::npos) {
                continue;
            }
            
            bool oRedir = false;
            bool aRedir = false;
            bool iRedir = false;
            string redirCmd = "";
            int redirInd = -1;
            if(commands[i].find(">>") != string::npos){
                redirCmd = commands[i];
                redirInd = commands[i].find(">>")+1;
                aRedir = true;
            }
            if(commands[i].find(">") != string::npos){
                oRedir = true;
                redirCmd = commands[i];
                redirInd = commands[i].find(">");
            }
            if(commands[i].find("<") != string::npos){
                redirCmd = commands[i];
                redirInd = commands[i].find("<");
                iRedir = true;
            }
            
            //echo
            commands[i] = commands[i].substr(pos);
            if(strcmp(commands[i].substr(0,4).c_str(), "echo") == 0 && !oRedir && !iRedir && !aRedir){
                cout<<commands[i].substr(5)<<endl;
                continue;
            }

            //Tokenize each command for tabs and spaces
            Tokenize(&commands[i][0], ip);
            int ip_size = ip.size();
            if(ip_size == 0) break;

            // for(int i=0; i<ip.size(); i++){
            //     cout<<ip[i]<<endl;
            // }
    
            //exit
            if(strcmp(ip[0].c_str(), "exit") == 0){
                saveHistory();
                return 0;
            }

            //background check
            string ip_last = ip[ip_size-1];
            if(strcmp(ip_last.c_str(),"&") == 0){
                isBackground = true;
                ip.pop_back();
                ip_size--;
            }

            //pwd
            if(strcmp(ip[0].c_str(), "pwd") == 0 && !oRedir && !iRedir && !aRedir){
                char *curr_dir;
                curr_dir = get_wd(path, buf_size);
                if(curr_dir)
                    cout<<curr_dir<<endl;
                continue;
            }

            //cd
            if(strcmp(ip[0].c_str(), "cd") == 0 && !oRedir && !iRedir && !aRedir){
                if(ip_size == 1){
                    string old = prev_dir;
                    prev_dir = curr_dir;
                    if(!ch_dir(home_dir)){
                        prev_dir = old;
                    }
                    continue;
                }
                if(ip_size>2){
                    cout<<"Invalid Arguments"<<endl;
                    break;
                }
                string cpath = get_wd(path,buf_size);
                if(strcmp(ip[1].c_str(), "..") == 0 && strcmp(cpath.c_str(),home_dir) == 0){
                    prev_dir = curr_dir;
                    cout<<cpath;
                }
                else if((strcmp(ip[1].c_str(),"~")==0) || (strcmp(ip[1].c_str(), "~/")==0)){
                    string old = prev_dir;
                    prev_dir = curr_dir;
                    if(!ch_dir(home_dir)){
                        prev_dir = old;
                    }
                }
                else if((strcmp(ip[1].c_str(),"-")==0)){
                    if(prev_dir.size()==0){
                        cerr<<"cd: OLDPWD not set"<<endl;
                    }
                    else{

                        string dirToChange = prev_dir;
                        prev_dir = curr_dir;
                        if(!ch_dir(dirToChange.c_str())){
                            prev_dir = dirToChange;
                        }
                    }
                }
                else{
                    string old = prev_dir;
                    prev_dir = curr_dir;
                    if(!ch_dir(ip[1].c_str())){
                        prev_dir = old;
                    }
                }
                //cout<<"prev dir: "<<prev_dir<<endl;
                continue;
            }

            //ls
            if(strcmp(ip[0].c_str(), "ls") == 0 && !oRedir && !iRedir && !aRedir) {
                bool showHidden = false;
                bool longList = false;
                vector<string> dirPath;
                for (int i = 1; i < ip_size; i++) {
                    if (ip[i] == "-a") {
                        showHidden = true;
                    } 
                    else if (ip[i] == "-l") {
                        longList = true;
                    }
                    else if (ip[i] == "-la") {
                        longList = true;
                        showHidden = true;
                    }
                    else if (ip[i] == "-al") {
                        longList = true;
                        showHidden = true;
                    }
                    else if(ip[i] == "~"){
                        dirPath.push_back(home);
                    }
                    else {
                        dirPath.push_back(ip[i]);
                    }
                }
                if(dirPath.size()==0){
                    dirPath.push_back(".");
                }
                listDir(dirPath,showHidden,longList);
                continue;
            }
            
            // pinfo command
            if (ip[0] == "pinfo" && !oRedir && !iRedir && !aRedir) {
                executePinfo(ip);
                continue;
            }

            //search
            if(strcmp(ip[0].c_str(),"search") == 0 && !oRedir && !iRedir && !aRedir){
                if(ip.size() != 2){
                    cout<<"Invalid Arguments"<<endl;
                    break;
                }
                cout<<boolalpha<<searchDir(".",ip[1])<<endl;
                continue;
            }

            //history
            if(strcmp(ip[0].c_str(),"history") == 0 && !oRedir && !iRedir && !aRedir){
                if(ip.size()==1){
                    displayHistory();
                    continue;
                }
                else{
                    string n = ip[1];
                    if(isInt(n)){
                        getHistory(stoi(n));
                        break;
                    }
                    else{
                        cout<<"Invalid Arguments"<<endl;
                        break;
                    }
                }
            }
            else{
                vector<string> redir_argv(2);
                if(oRedir || iRedir || aRedir){
                    parse_redir(ip, redir_argv);
                }
                execCmd(ip, isBackground, oRedir, aRedir, iRedir, redir_argv);
            }
        }
    }
    saveHistory();
    //disableRawMode();
    return 0;
}
