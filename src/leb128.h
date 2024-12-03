#ifndef __LEB128_H__
#define __LEB128_H__
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "log.h"

uint32_t DecodeUnsignedLeb128(uint8_t** data);

uint8_t* EncodeUnsignedLeb128(uint8_t* dest, uint32_t value);

/*
example:

int main() {
    uint8_t* dest = (uint8_t*)malloc(sizeof(uint8_t));
    EncodeUnsignedLeb128(dest, 12345);
    uint32_t res = DecodeUnsignedLeb128(&dest);
    DEBUG("%d\n", res);
}
*/

#endif