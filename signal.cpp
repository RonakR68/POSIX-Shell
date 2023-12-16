#include <iostream>
#include <csignal>
#include <unistd.h>
using namespace std;
pid_t foreground = -1;

void signalCtrlC(int sigNum){
    if (sigNum == SIGINT){
        if (foreground != -1){
            kill(foreground, SIGINT);
        }
    }
}

void signalCtrlZ(int sigNum){
    if (foreground != -1){
        kill(foreground, SIGTSTP);
        foreground = -1;
    }
}



    

