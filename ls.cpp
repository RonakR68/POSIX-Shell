#include<iostream>
#include<string>
#include<vector>
#include<sys/types.h>
#include<dirent.h>
#include<sys/stat.h> 
#include<pwd.h>
#include<grp.h>
#include<iomanip>
using namespace std;

void printDetails(string path){
    struct stat st;
    if (stat(path.c_str(), &st) < 0) {
        cerr<<"Error while using stat."<<endl;
        return;
    }
    string permissions;
    mode_t mode = st.st_mode;
    if (S_ISDIR(mode)) permissions += "d"; //directory
    else if (S_ISREG(mode)) permissions += "-"; //regular
    else if (S_ISBLK(mode)) permissions += "b"; //block
    else if (S_ISCHR(mode)) permissions += "c"; //character
    else if (S_ISLNK(mode)) permissions += "l"; //link
    else if (S_ISFIFO(mode)) permissions += "p"; //pipe
    else if (S_ISSOCK(mode)) permissions += "s"; //socket
    else permissions += "-";

    permissions += (mode & S_IRUSR) ? "r" : "-";
    permissions += (mode & S_IWUSR) ? "w" : "-";
    permissions += (mode & S_IXUSR) ? "w" : "-";
    permissions += (mode & S_IRGRP) ? "r" : "-";
    permissions += (mode & S_IWGRP) ? "w" : "-";
    permissions += (mode & S_IXGRP) ? "w" : "-";
    permissions += (mode & S_IROTH) ? "r" : "-";
    permissions += (mode & S_IWOTH) ? "w" : "-";
    permissions += (mode & S_IXOTH) ? "w" : "-";

    nlink_t no_links = st.st_nlink;

    struct passwd* pw = getpwuid(st.st_uid);
    struct group* gr = getgrgid(st.st_gid);
    string owner = pw ? pw->pw_name : to_string(st.st_uid);
    string group = gr ? gr->gr_name : to_string(st.st_gid);


    char buff[100];
    strftime(buff, sizeof(buff), "%b %d %H:%M", localtime(&st.st_mtime));

    cout<<left<<setw(10)<<permissions<<" ";
    cout<<right<<setw(3)<<no_links<<" ";
    cout<<left<<setw(8)<<owner<<" ";
    cout<<left<<setw(8)<<group<<" ";
    cout<<left<<setw(8)<<st.st_size<<" ";
    cout<<left<<setw(12)<<buff <<" ";
    cout<<left<<path<<endl;
}

void listDir(vector<string> &dirPath, bool showHidden, bool longList){
    struct stat fst;
    for(int i=0; i<dirPath.size(); i++){
        //cout<<"checking: "<<dirPath[i]<<endl;
        if (stat(dirPath[i].c_str(), &fst) == 0 && S_ISDIR(fst.st_mode)){
            //directory
            DIR *dir = opendir(dirPath[i].c_str());
            if (dir == nullptr) {
                cerr << "Error while opening directory."<<endl;
                return;
            }
            if(dirPath.size()>1){
                cout<<dirPath[i]<<":"<<endl;
            }
            dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (!showHidden && entry->d_name[0] == '.') {
                    continue;
                }
                if (longList) {
                    printDetails(dirPath[i] + + "/" + entry->d_name);
                }
                else {
                    cout<<entry->d_name<<endl;
                }
            }
            if(dirPath.size()>1 && i<dirPath.size()-1)
                cout<<endl;
            closedir(dir);
        }
        else{
            if(S_ISREG(fst.st_mode)){
                //file
                if(longList){
                    //cout<<setw(200);
                    printDetails(dirPath[i]);
                }
                else{
                    cout<<dirPath[i]<<endl;
                }
            }
            else{
                cerr << "Error: No such file or directory"<<endl;
            }
        }
    }
}