#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t DecodeUnsignedLeb128(uint8_t** data) {
  uint8_t* ptr = *data;
  int result = *(ptr++);
  if (result > 0x7f) {
    int cur = *(ptr++);
    result = (result & 0x7f) | ((cur & 0x7f) << 7);
    if (cur > 0x7f) {
      cur = *(ptr++);
      result |= (cur & 0x7f) << 14;
      if (cur > 0x7f) {
        cur = *(ptr++);
        result |= (cur & 0x7f) << 21;
        if (cur > 0x7f) {
          // Note: We don't check to see if cur is out of range here,
          // meaning we tolerate garbage in the four high-order bits.
          cur = *(ptr++);
          result |= cur << 28;
        }
      }
    }
  }
  *data = ptr;
  return (uint32_t)result;
}

uint8_t* EncodeUnsignedLeb128(uint8_t* dest, uint32_t value) {
  uint8_t out = value & 0x7f;
  value >>= 7;
  while (value != 0) {
    *dest++ = out | 0x80;
    out = value & 0x7f;
    value >>= 7;
  }
  *dest++ = out;
  return dest;
}

int main() {
    uint8_t* dest = (uint8_t*)malloc(sizeof(uint8_t));
    EncodeUnsignedLeb128(dest, 12345);
    uint32_t res = DecodeUnsignedLeb128(&dest);
    printf("%d\n", res);
}