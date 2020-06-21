#include "../run2.h"

void grad_case10(float (&dA)[8][8], float (&dB)[10][8]) {
  float tmp0[10][8];
  for (int i = 0; i < 8; ++i){
    for (int j = 0; j < 8; ++j){
      tmp0[i][j] = 0;
      tmp0[i][j] = (tmp0[i][j] + (dA[i][j] * ((((1 + 1) + 1) * 1.07794e+09) / (1.07794e+09 * 1.07794e+09))));
    }
  }
  for (int i = 0; i < 8; ++i){
    for (int j = 0; j < 8; ++j){
      dB[i][j] = tmp0[i][j];
    }
  }
}
