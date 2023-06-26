#include<stdlib.h>
#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    char* path = "/home/aosp/Project/dex2fill/item_9771204.item";
    FILE *item_fd = fopen(path, "r");
    char* line_count = (char*)malloc(0x1000);
    char* name = (char*)malloc(0x100);
    char* count = (char*)malloc(0x1000);
    int item_idx;
    while(1){
        fscanf(item_fd, "%[^:] : %d : %s\n", name, &item_idx, count);
        if(item_idx == -1) break;
        printf("%s - %d - %s\n", name, item_idx, count);
        item_idx = -1;
    }
    return 0;
}