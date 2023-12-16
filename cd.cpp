#include<unistd.h>
#include<iostream>
using namespace std;

bool ch_dir(const char *path)
{
	if(chdir(path)<0){
		perror("Error while executing cd");
		return false;
	}
	return true;
}