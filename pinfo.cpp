#include "headers.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>


using namespace std;

// Function to get virtual memory usage from the /proc filesystem
unsigned long getVirtualMemory(pid_t pid){
    unsigned long virtualMemory = 0;
    ifstream statFile;
    stringstream statFilePath;
    statFilePath << "/proc/" << pid << "/statm";
    statFile.open(statFilePath.str());

    if(statFile.is_open()){
        // The first value in statm represents the total program size in pages
        statFile >> virtualMemory;
        statFile.close();
    }
    return virtualMemory;
}

// Function to get process status from the /proc filesystem
string getProcessStatus(pid_t pid){
    string status;
    ifstream statusFile;
    stringstream statusFilePath;
    statusFilePath << "/proc/" << pid << "/status";
    statusFile.open(statusFilePath.str());

    if(statusFile.is_open()){
        string line;
        while(getline(statusFile, line)){
            size_t startPos = line.find_first_not_of(" \t");
            if(startPos != string::npos && line.substr(startPos, 6) == "State:"){
                string state = line.substr(startPos + 6);
                //cout<<"state is: "<<state<<endl;
                int i=0;
                while(i<state.size()){
                    if(state[i]=='\t'||state[i]==' '){
                        i++;
                        continue;
                    }
                    else
                        break;
                }
                //cout<<"i is: "<<i<<endl;
                // Interpret process state
                if(!state.empty() && (state[i] == 'R' || state[i] == 'r')){
                    status = "R";
                } 
                else if(!state.empty() && (state[i] == 'S' || state[i] == 's')){
                    status = "S";
                } 
                else if(!state.empty() && state[i] == 'Z'){
                    status = "Z";
                } 
                else{
                    status = "Unknown";
                }

                // Add '+' if the process is in the foreground
                if(state.size() > 1 && state[1] == '+'){
                    status += "+";
                }
                break;
            }
        }
        statusFile.close();
    }
    return status;
}


// Function to get executable path from /proc filesystem
string getExecutablePath(pid_t pid){
    string execPath;
    ifstream exeFile;
    stringstream exeFilePath;
    exeFilePath << "/proc/" << pid << "/exe";
    char pathBuffer[1024];
    ssize_t len = readlink(exeFilePath.str().c_str(), pathBuffer, sizeof(pathBuffer) - 1);
    if(len != -1){
        pathBuffer[len] = '\0';
        execPath = pathBuffer;
    }
    return execPath;
}

// Function to print process information
void printProcessInfo(pid_t pid){
    string processStatus = getProcessStatus(pid);
    string executablePath = getExecutablePath(pid);

    cout << "pid -- " << pid << endl;
    cout << "Process Status -- " << processStatus << endl;
    cout << "memory -- " << getVirtualMemory(pid) << " {Virtual Memory}" << endl;
    cout << "Executable Path -- " << executablePath << endl;
}

// Function to handle pinfo command
void handlePinfo(char* args[], int n){
    if(n == 1){
        // If no PID is provided, print information for the shell process
        pid_t shellPid = getpid();
        printProcessInfo(shellPid);
    } 
    else if(n == 2){
        // If PID is provided, print information for the specified process
        pid_t pid;
        try{
            pid = stoi(args[1]);
        } 
        catch (const invalid_argument& e){
            cerr << "Invalid PID format." << endl;
            return;
        }
        printProcessInfo(pid);
    } 
    else{
        cout << "Invalid usage of pinfo. Use 'pinfo' to get information about the shell process or 'pinfo <pid>' for a specific process." << endl;
    }
}
