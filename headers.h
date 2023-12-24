#include<iostream>
#include<vector>
#include<string>
#include<unistd.h> //hostname, getcwd, chdir, execvp
#include<string.h> //strtok
#include<sys/types.h> //file type
#include<dirent.h> //dirent
#include<sys/stat.h> //stat
#include<pwd.h> //getpwuid
#include<grp.h> //getgrpid
#include<iomanip> //setw
#include<sys/wait.h> //wait
#include <csignal> //signal
#include<fcntl.h> //history file
#include<sstream> //history file
#include <termios.h> //up arrow key
#include <cstring>
using namespace std;

//history
extern vector<string> history;
extern int store_size;
extern int display_size;
void addHistory(string command);
void displayHistory();
void getHistory(int num);
void loadHistory();
void saveHistory();
char* get_wd(char *buf, size_t size); //pwd
bool ch_dir(const char *path); //cd
//ls
void printDetails(string path);
void listDir(vector<string> &dirPath, bool showHidden, bool longList);
bool searchDir(string dirPath, string searchTarget); //search
//signal
extern pid_t foreground;
void signalCtrlC(int sigNum);
void signalCtrlZ(int sigNum);
//pinfo
void handlePinfo(char* args[], int n);
