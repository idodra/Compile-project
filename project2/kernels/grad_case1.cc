#include "../run2.h"

void grad_case1(float (&B)[4][16], float (&dC)[4][16], float (&dA)[4][16]) {
  float tmp0[4][16];
  for (int i = 0; i < 4; ++i){
    for (int j = 0; j < 16; ++j){
      tmp0[i][j] = 0;
      tmp0[i][j] = (tmp0[i][j] + (dC[i][j] * B[i][j]));
    }
  }
  for (int i = 0; i < 4; ++i){
    for (int j = 0; j < 16; ++j){
      dA[i][j] = tmp0[i][j];
    }
  }
}
