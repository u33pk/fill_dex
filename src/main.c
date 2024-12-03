#include "dex.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include "log.h"

int padding_file(char* dex_path);
int reduction(char* dex_path, char* item_path, int fill_mod, int pad_mod);
void display_help(char* arg0);

int main(int argc, char* argv[], char* envp[]){
    int opt;
    const char *short_options = "m:p:d:i:h";
    int option_index = 0;
    struct option long_options[] = {
        {"fill_mod", required_argument, 0, 'm' },
		{"pad_switch", required_argument, 0,  'p' },
		{"dex_path", required_argument, 0, 'd' },
		{"item_path", required_argument, 0, 'i' },
        {"help", no_argument, 0, 'h' },
		{0, 0, 0, 0 }
    };
    char* dex_path;
    char* item_path;
    int fill_mod;       // 0: 使用偏移  1: 使用索引
    int pad_switch;     // 0: 不填充    1: 填充
    while((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1){
        switch (opt) {
            case 'm':
                if(strcmp(optarg, "offset") == 0)
                    fill_mod = 0;
                else if (strcmp(optarg, "index" ) == 0)
                    fill_mod = 1;
                else{
                    DEBUG("-m --fill_mod: offset / index\n");
                    exit(0);
                }
                break;
            case 'p':
                if(strcmp(optarg, "off") == 0)
                    pad_switch = 0;
                else if (strcmp(optarg, "on") == 0)
                    pad_switch = 1;
                else{
                    DEBUG("-p --pad_switch: on / off\n");
                    exit(0);
                }
                break;
            case 'd':
                dex_path = optarg;
                break;
            case 'i':
                item_path = optarg;
                break;
            case 'h':
                display_help(argv[0]);
                return 0;
            default:
                display_help(argv[0]);
                return 0;
        }
    }
    DEBUG("arg: %d %d %s %s\n", fill_mod, pad_switch, dex_path, item_path);
    reduction(dex_path, item_path, fill_mod, pad_switch);
    return 0;
}


int reduction(char* dex_path, char* item_path, int fill_mod, int pad_switch) {
    int padding_len = 0;
    if(pad_switch == 1)
        padding_len = padding_file(dex_path);
    int dex_fd = open(dex_path, O_RDWR);
    struct stat dex_stat;
    fstat(dex_fd, &dex_stat);
    struct DexHeader* dex_head = (struct DexHeader*)mmap(NULL, dex_stat.st_size , PROT_READ|PROT_WRITE, MAP_SHARED , dex_fd , 0);
    if(pad_switch == 1){
        if(padding_len == 0)
            dex_head->fileSize = dex_stat.st_size*2;
        else
            dex_head->fileSize = dex_stat.st_size + padding_len;
    }
    close(dex_fd);
    FILE* item_file = fopen(item_path, "r");
    fill_from_file(dex_head, item_file, fill_mod);
    int sync_res = msync((void*)dex_head, dex_stat.st_size, MS_SYNC);
    DEBUG("sync result: %d\n", sync_res);
    munmap((void*)dex_head, dex_stat.st_size);
    fclose(item_file);
    return 0;
}


int padding_file(char* dex_path){
    FILE* pad_file = fopen(dex_path, "ab+");
    struct stat pad_stat;
    fstat(pad_file->_fileno, &pad_stat);
    size_t target_len = (pad_stat.st_size/4096 + 1) * 4096;
    char *z_block = (char*)malloc(target_len);
    memset(z_block, 0x00, target_len);
    if(pad_file != NULL){
        fwrite(z_block, 4096, pad_stat.st_size/4096 + 1, pad_file);
    }
    fflush(pad_file);
    fclose(pad_file);
    return target_len;
}

void display_help(char* arg0){
    printf("usage: %s [arguments]\n", arg0);
    printf("\t-m or --fill_mod    index/offset\n");
    printf("\t-p or --pad_switch  on/off\n");
    printf("\t-d or --dex_path    dex path\n");
    printf("\t-i or --item_path   item path\n");
    printf("\t-h or --help\n");
}