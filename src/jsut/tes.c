#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *flt_to_str() {
  long double arg = 0;  // значение вещественного аргумента

  long double mantissa = 0.0000756589367;  // значение мантиссы аргумента
  int n = 0;  // порядок степени аргумента в экспоненциальной нотации
  // вычислить мантиссу и порядок степени аргумента
  while (fabsl(mantissa) >= 10) {
    mantissa /= 10;
    n++;
  }
  while (0 < fabsl(mantissa) && fabsl(mantissa) < 1) {
    mantissa *= 10;
    n--;
  }
}

int main(int argc, char const *argv[]) {
  // long double left = 0, right = 0;
  // long double number = 9999.999990000000252621248;
  // int pos = 0;
  // char result[256] = {0};
  // right = modfl(number, &left);
  // char *str3 = "\0 Test \0 Test \0 Test \0 Test \0";

  // printf("%s\n", str3);
  // printf("%d", (int)strlen(str3));
  // // printf("%Lf", left);
  flt_to_str();
  return 0;
}
// 823631075973858560220268288082.0
// 823631075973858582992951181312.0