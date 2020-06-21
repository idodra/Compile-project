#include "../run2.h"

void grad_case3(float (&B)[16][16], float (&dC)[4][16], float (&dA)[4][16]) {
  float tmp0[4][16];
  for (int i = 0; i < 4; ++i){
    for (int k = 0; k < 16; ++k){
      tmp0[i][k] = 0;
      for (int j = 0; j < 16; ++j){
        tmp0[i][k] = (tmp0[i][k] + (dC[i][j] * B[k][j]));
      }
    }
  }
  for (int i = 0; i < 4; ++i){
    for (int k = 0; k < 16; ++k){
      dA[i][k] = tmp0[i][k];
    }
  }
}
