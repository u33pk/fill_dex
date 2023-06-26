#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t DecodeUnsignedLeb128(uint8_t** data);

uint8_t* EncodeUnsignedLeb128(uint8_t* dest, uint32_t value);

/*
example:

int main() {
    uint8_t* dest = (uint8_t*)malloc(sizeof(uint8_t));
    EncodeUnsignedLeb128(dest, 12345);
    uint32_t res = DecodeUnsignedLeb128(&dest);
    printf("%d\n", res);
}
*/