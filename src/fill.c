#include "fill.h"

void fill_code(struct DexCode* dest, uint8_t* src, int src_len){
    memcpy((void*)dest, (void*)src, src_len);
}

void fill_from_file(struct DexHeader* dex_mem, FILE* item_file) {
    method_count_b64 = (char*)mmap(NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    method_count = (char*)mmap(NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    while (1)
    {
        uint32_t method_idx;
        int count_len;
        int read_len = fscanf(item_file, "%d : %s", &method_idx, method_count_b64);
        if(read_len > 0){
            count_len = base64_decode(method_count_b64, strlen(method_count_b64), method_count);
            fill_from_line(dex_mem, method_idx, method_count, count_len);
        }
        memset(method_count_b64, 0, 0x10000);
        memset(method_count, 0, 0x10000);
        if(read_len <= 0){
            break;
        }
    }
    munmap(method_count_b64, 0x10000);
    munmap(method_count, 0x10000);
}

void fill_from_line(struct DexHeader* dex_mem, uint32_t method_idx, uint8_t* method_count, int count_len){
    struct DexCode* code = find_method(dex_mem, method_idx);
    if(code == NULL){
        printf("not found method: %d\n", method_idx);
    } else {
        fill_code(code, method_count, count_len);
    }
}