#include "dex.h"
#include "log.h"
#include "base64.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

char* method_count_b64;
char* method_count;

/// @brief 向目标地址填充opcode
/// @param dest dex_mem中的目标code地址
/// @param src 脱壳下来的函数内容
/// @param src_len 脱壳下来的函数内容长度
void fill_code(struct DexCode * dest, uint8_t* src, int src_len);

/// @brief 从指定的item文件中向指定的dex中填充code
/// @param dex_mem 指定的dex内存文件
/// @param item_file 指定的文件
/// @param mod 修复模式, 0: 使用偏移, 1: 使用索引
void fill_from_file(struct DexHeader* dex_mem, FILE* item_file, int mod);

/// @brief 向指定的dex中的指定的method中写入脱下来的code
/// @param dex_mem 指定的dex内存
/// @param method_idx 指定的method
/// @param method_count 脱下来的item code
/// @param count_len item code的长度
void fill_from_line(struct DexHeader* dex_mem, uint32_t method_idx, uint8_t* method_count, int count_len);

/// @brief 向指定dex中的偏移写入脱下来的code
/// @param dex_mem 指定的dex内存
/// @param method_off 指定的偏移
/// @param method_count 脱下来的item code
/// @param count_len item code的长度
void fill_from_line_offmod(struct DexHeader* dex_mem, uint32_t method_off, uint8_t* method_count, int count_len);