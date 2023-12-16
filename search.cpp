#include<iostream>
#include<string.h>
#include<sys/stat.h>
#include<dirent.h>
using namespace std;

bool searchDir(string dirPath, string searchTarget){
    DIR *dir = opendir(dirPath.c_str()); //open dir stream for current dir
    if(dir == nullptr) 
        return false;
    dirent *entry;
    while(1){
        entry = readdir(dir); //get next subdir/file
        if(entry == nullptr)
            break;
        if(strcmp(entry->d_name,"..")==0 || strcmp(entry->d_name,".")==0)
            continue;
        string path = dirPath + "/" + entry->d_name;
        struct stat st;
        if(stat(path.c_str(), &st) < 0){
            closedir(dir);
            return false;
        }
        if(S_ISREG(st.st_mode) && strcmp(entry->d_name, searchTarget.c_str()) == 0){
            //file found
            closedir(dir);
            return true;
        }
        if(S_ISDIR(st.st_mode)){
            if(strcmp(entry->d_name, searchTarget.c_str()) == 0)
                return true;
            //recursively search in subdirectories
            if(searchDir(path, searchTarget))
                return true;
            else
                continue;
        }
    }
    closedir(dir);
    return false;
}
