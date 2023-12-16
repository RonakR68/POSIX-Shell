#include<unistd.h>

char* get_wd(char *buf, size_t size){ 
return getcwd(buf,100);
}
