#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *s21_itoa(long long value, char *str, int radix) {
  char *buffer = str;
  value = llabs(value);
  do {
    *buffer = value % radix + (value % radix < 10 ? 48 : 87);
    ++buffer;
    value /= radix;
  } while (value != 0);
  *buffer = '\0';
  --buffer;

  for (int i = 0; (str + i) < (buffer - i); i++) {
    char temp = *(str + i);
    *(str + i) = *(buffer - i);
    *(buffer - i) = temp;
  }
  return str;
}

int main(int argc, char const *argv[]) {
  long double a = 0, b = 0;
  long double value = (double)9325781235683689988.;

  char result[256] = {0};

  s21_itoa(llroundl(value), result, 10);
  printf("%s", result);
  return 0;
}
// 823631075973858560220268288082.0
// 823631075973858582992951181312.0
