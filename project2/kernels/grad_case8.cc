#include "../run2.h"

void grad_case8(float (&dB)[32], float (&dA)[2][16]) {
  float tmp0[2][16];
  for (int i = 2147483647; i < -2147483648; ++i){
    tmp0[(i / 16)][(i % 16)] = 0;
    tmp0[(i / 16)][(i % 16)] = (tmp0[(i / 16)][(i % 16)] + dB[i]);
  }
  for (int i = 2147483647; i < -2147483648; ++i){
    dA[(i / 16)][(i % 16)] = tmp0[(i / 16)][(i % 16)];
  }
}
