#include "dex.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include<sys/stat.h>
// #include<uninstd.h>

int main() {
    
    int dex_fd = open("/home/aosp/Project/dex2fill/classes.dex", O_RDWR);
    struct stat dex_stat;
    fstat(dex_fd, &dex_stat);
    struct DexHeader* dex_head = (struct DexHeader*)mmap(NULL, dex_stat.st_size , PROT_READ|PROT_WRITE, MAP_SHARED , dex_fd , 0);
    struct DexCode * code = find_method(dex_head, 22743);
}