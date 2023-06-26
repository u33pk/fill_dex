#include "dex.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "base64.h"

int main() {
    
    int dex_fd = open("/home/aosp/Project/dex2fill/classes.dex", O_RDWR);
    struct stat dex_stat;
    fstat(dex_fd, &dex_stat);
    struct DexHeader* dex_head = (struct DexHeader*)mmap(NULL, dex_stat.st_size , PROT_READ|PROT_WRITE, MAP_SHARED , dex_fd , 0);
    // printf("class def size: %d\n", dex_head->classDefsSize);

    // uint32_t method_len = 0;
    // struct DexCode* code_item = get_method_off(dex_head, 100, &method_len);
    // printf("%d\n", code_item->registersSize);
    // printf("%d\n", code_item->insnsSize);
    struct DexCode * code = find_method(dex_head, 22743);
    
    return 0;
}