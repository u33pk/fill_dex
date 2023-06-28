#include "dex.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include<sys/stat.h>
#include <unistd.h>
#include "fill.h"

int main(int argc, char* args[], char* envp[]) {
    
    int dex_fd = open("/home/aosp/Project/dex2fill/classes3.dex", O_RDWR);
    FILE* item_file = fopen("/home/aosp/Project/dex2fill/item_12794744.item", "r");
    struct stat dex_stat;
    fstat(dex_fd, &dex_stat);
    struct DexHeader* dex_head = (struct DexHeader*)mmap(NULL, dex_stat.st_size , PROT_READ|PROT_WRITE, MAP_SHARED , dex_fd , 0);
    close(dex_fd);
    fill_from_file(dex_head, item_file);
    int sync_res = msync((void*)dex_head, dex_stat.st_size, MS_SYNC);
    printf("sync result: %d\n", sync_res);
    munmap((void*)dex_head, dex_stat.st_size);
    fclose(item_file);
    return 0;
}