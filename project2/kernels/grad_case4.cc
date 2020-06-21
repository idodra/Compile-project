#include "../run.h"

void grad_case4(float (&B)[16][32], float (&C)[32][32], float (&dA)[16][32], float (&dB)[16][32], float (&dC)[32][32]) {
  float tmp0[16][32];
  for (int i = 0; i < 16; ++i){
    for (int k = 0; k < 32; ++k){
      tmp0[i][k] = 0;
      for (int j = 0; j < 32; ++j){
        tmp0[i][k] = (tmp0[i][k] + (dA[i][j] * C[k][j]));
      }
    }
  }
  for (int i = 0; i < 16; ++i){
    for (int k = 0; k < 32; ++k){
      dB[i][k] = tmp0[i][k];
    }
  }
  float tmp1[32][32];
  for (int k = 0; k < 32; ++k){
    for (int j = 0; j < 32; ++j){
      tmp1[k][j] = 0;
      for (int i = 0; i < 16; ++i){
        tmp1[k][j] = (tmp1[k][j] + (dA[i][j] * B[i][k]));
      }
    }
  }
  for (int k = 0; k < 32; ++k){
    for (int j = 0; j < 32; ++j){
      dC[k][j] = tmp1[k][j];
    }
  }
}
