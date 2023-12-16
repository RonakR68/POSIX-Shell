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

//execute other commands
void execCmd(vector<string> &v, bool background){
    pid_t pid = fork();
    if(pid == 0){
        //child process
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

int main(){
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

            //echo
            commands[i] = commands[i].substr(pos);
            if(strcmp(commands[i].substr(0,4).c_str(), "echo") == 0){
                cout<<commands[i].substr(5)<<endl;
                continue;
            }

            //Tokenize each command for tabs and spaces
            Tokenize(&commands[i][0], ip);
            int ip_size = ip.size();
            if(ip_size == 0) break;
    
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
            if(strcmp(ip[0].c_str(), "pwd") == 0){
                char *curr_dir;
                curr_dir = get_wd(path, buf_size);
                if(curr_dir)
                    cout<<curr_dir<<endl;
                continue;
            }

            //cd
            if(strcmp(ip[0].c_str(), "cd") == 0){
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
            if(strcmp(ip[0].c_str(), "ls") == 0) {
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
            if (ip[0] == "pinfo") {
                executePinfo(ip);
                continue;
            }

            //search
            if(strcmp(ip[0].c_str(),"search") == 0){
                if(ip.size() != 2){
                    cout<<"Invalid Arguments"<<endl;
                    break;
                }
                cout<<boolalpha<<searchDir(".",ip[1])<<endl;
                continue;
            }

            //history
            if(strcmp(ip[0].c_str(),"history") == 0){
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
                execCmd(ip, isBackground);
            }
        }
    }
    saveHistory();
    return 0;
}
