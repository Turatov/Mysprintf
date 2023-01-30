#include <string.h>

#include "s21_string.h"
void *s21_trim(const char *src, const char *trim_chars) {
  char *newSrc = calloc(strlen(src), sizeof(char));
  char *newEnd = calloc(strlen(src), sizeof(char));
  size_t len = strlen(src);
  size_t begin = 0;
  size_t end = 0;
  size_t delCount = 0;
  newSrc = (char *)src;
  for (size_t v = 0; src[v] != '\0'; v++) {
    if (!strchr(trim_chars, src[v])) {
      begin = v;
      break;
    }
  }
  for (; len > 0; len--) {
    if (!strchr(trim_chars, src[len])) {
      end = len;
      break;
    }
  }

  int last = 0;
  for (size_t i = 0; i < begin; i++) {
    for (size_t j = 0; trim_chars[j] != '\0'; j++) {
      if (src[i] == trim_chars[j]) {
        last = i;
        for (size_t l = 0; src[i + l] != '\0'; l++) {
          newEnd[l] = newSrc[(i + l + 1) - delCount];
        }
        if (newSrc[(last + 1) - delCount] == '\0') {
          newSrc[last - delCount] = '\0';
          delCount++;
        } else {
          newSrc = s21_insert(newSrc, newEnd, last - delCount);
          delCount++;
        }
      }
    }
  }
  newSrc[end - delCount + 1] = '\0';
  return newSrc;
}

// int left_side(const char *src, const char *trim_chars, int last) {
//   int res = 0;
//   int size = s21_strlen(trim_chars);
//   for (int i = 0; i < size; i++) {
//     if (src[last] == trim_chars[i]) res = 1;
//   }
//   return res;
// }

// int right_side(const char *src, const char *trim_chars, int last) {
//   int res = 0;
//   last--;
//   int size = s21_strlen(trim_chars);
//   for (int i = 0; i < size; i++) {
//     if (src[last] == trim_chars[i]) res = 1;
//   }
//   return res;
// }

// void *s21_trim(const char *src, const char *trim_chars) {
//   char *arr = s21_NULL;
//   if (src) {
//     if (trim_chars && trim_chars[0]) {
//       arr = calloc(s21_strlen(src) + 1, sizeof(char));
//       s21_size_t begin = 0, last = s21_strlen(src);
//       while (left_side(src, trim_chars, begin)) {
//         begin++;
//       }
//       if (begin != last) {
//         while (right_side(src, trim_chars, last)) last--;
//       } else {
//         arr[0] = '\0';
//       }
//       for (int i = 0; begin < last; i++) {
//         arr[i] = src[begin];
//         begin++;
//       }
//     } else {
//       arr = s21_trim(src, "\t\n ");
//     }
//   }
//   return arr;
// }
// #define test
// #ifdef test
// int main(int argc, char const *argv[]) {
//   int h = 0;
//   int m = 0;

//   char s1[] = " wtf ";
//   char *s3 = S21_NULL;
//   char *s4 = " wtf ";
//   char *s2 = s21_trim(s1, s3);
//   printf("s21 === *%s* = %d=\n", s1, h);
//   printf("man === *%s* = %d=\n", s1, m);

//   return 0;
// }
// #endif
