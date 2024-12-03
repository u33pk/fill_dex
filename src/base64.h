#ifndef BASE64_H
#define BASE64_H

#define BASE64_ENCODE_OUT_SIZE(s) ((unsigned int)((((s) + 2) / 3) * 4 + 1))
#define BASE64_DECODE_OUT_SIZE(s) ((unsigned int)(((s) / 4) * 3))

#include<stdio.h>
#include "log.h"

/// @brief 对指定内容进行base64编码
/// @param in 要编码的内容
/// @param inlen 内容的长度
/// @param out 编码后的内容
/// @return 编码后的内容长度
unsigned int base64_encode(const unsigned char *in, unsigned int inlen, char *out);

/// @brief base64 解码
/// @param in 输入的base64内容
/// @param inlen base64内容长度
/// @param out 解码内容
/// @return out解码内容长度
unsigned int base64_decode(const char *in, unsigned int inlen, unsigned char *out);

/// @brief 输出 bytes hexdump
/// @param buf 指定的bytes
/// @param len 要输出的长度, 小于 buf 的长度
void hex_dump(const char *buf, int len);

#endif /* BASE64_H */