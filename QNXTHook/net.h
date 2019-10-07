#pragma once

#include <stdint.h>

template <typename T>
T UnpackData(uint8_t *data, uint32_t bitSize, uint32_t bitIndex,
             uint32_t *outBitIndex) {
  T unpacked = {0};

  int shiftBy = bitSize;
  int i = bitIndex >> 3;

  *outBitIndex += bitSize;

  uint8_t bitOffset = 8 - (bitIndex & 7);
  uint8_t mask = (1 << bitOffset) - 1;

  while (shiftBy >= bitOffset) {
    shiftBy -= bitOffset;
    unpacked += ((data[i] & mask) << shiftBy);
    i++;

    bitOffset = 8;
    mask = 0xFF;
  }

  if (shiftBy == bitOffset) {
    return unpacked + (data[i] & mask);
  }
  return unpacked + (data[i] >> (bitOffset - shiftBy)) & ((1 << bitSize) - 1);
}