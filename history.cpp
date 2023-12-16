#include<iostream>
#include<vector>
#include<unistd.h>
#include<fcntl.h>
#include<sstream>
using namespace std;

int store_size = 20;
int display_size = 10;
vector<string> history;

void addHistory(string command){
    if(history.size() >= store_size){
        history.erase(history.begin());
    }
    history.push_back(command);
}

void displayHistory(){
    int curr_size = history.size(), ind;
    if(display_size >= curr_size){
        ind = 0;
    }
    else{
        ind = curr_size-display_size;
    }
    for(int i=ind; i<curr_size; i++){
        cout<<history[i]<<endl;
    }
}

void getHistory(int num){
    int curr_size = history.size(), ind;
    if(num >= curr_size){
        ind = 0;
    }
    else{
        ind = curr_size-num;
    }
    for(int i=ind; i<curr_size; i++){
        cout<<history[i]<<endl;
    }
}

void loadHistory(){
    //cout<<"loading history"<<endl;
    int history_fd = open("history.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (history_fd == -1) {
        cerr<<"Error opening history file"<<endl;
        return;
    }
    char buffer[8192];
    int bytesRead;
    while((bytesRead=read(history_fd, buffer, sizeof(buffer))) > 0){
        string fileData(buffer, bytesRead);
        istringstream hStream(fileData);
        string cmd;
        while (getline(hStream, cmd)){
            history.push_back(cmd);
        }
    }
    close(history_fd);
}

void saveHistory(){
    int history_fd = open("history.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (history_fd == -1) {
        cerr<<"Error opening history file"<<endl;
        return;
    }
    for (string cmd:history){
        string cmdLine = cmd + "\n";
        write(history_fd, cmdLine.c_str(), cmdLine.size());
    }
    close(history_fd);
}
