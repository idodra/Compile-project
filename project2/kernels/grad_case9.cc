#include "../run.h"

void grad_case9(float (&dB)[4][6], float (&dA)[4]) {
  float tmp0[4];
  for (int i = 0; i < 4; ++i){
    tmp0[i] = 0;
    for (int j = 0; j < 6; ++j){
      tmp0[i] = (tmp0[i] + dB[i][j]);
    }
  }
  for (int i = 0; i < 4; ++i){
    dA[i] = tmp0[i];
  }
}
