#include <wchar.h>

#include "ctype.h"
#include "s21_string.h"

char *s22str_to_str(va_list *args, char *str, s21_FORMAT *cs);

char *strchrncat(char *str, char ch, int n, int f) {
  if (n <= 0) return str;
  s21_size_t m = s21_strlen(str);
  if (f) {  // дублировать символ ch в конец строки n раз
    s21_memset(str + m, ch, n);
    str[m + n] = '\0';
  } else {  // дублировать символ ch в начало строки n раз
    s21_memmove(str + n, str, m + 1);
    s21_memset(str, ch, n);
  }
  return str;
}  // конец strchrncat
char *s21_itoa(long long value, char *str, int radix) {
  char *buffer = str;
  // if (value < 0) value *= -1;
  value = llabs(value);  // преобразовывать в положительное число
  do {  // вычленить и заменить цифру на соответствующую букву в своей
        // системе счисления ('0'=48 ... '9'=57, 'a'=97 ... 'f'=102)
    *buffer = value % radix + (value % radix < 10 ? 48 : 87);
    ++buffer;
    value /= radix;
  } while (value != 0);
  *buffer = '\0';
  --buffer;
  // перевернуть строку
  for (int i = 0; (str + i) < (buffer - i); i++) {
    char temp = *(str + i);
    *(str + i) = *(buffer - i);
    *(buffer - i) = temp;
  }
  return str;
}  // конец s21_itoa

// преобразование строки в целое число; возвращается ноль,
// eсли в начале строки стоит символ, не являющейся цифрой
int s21_atoi(char *str) {
  int sum = 0;
  s21_size_t n = s21_strspn(str, "1234567890");  // количество цифр в числе
  for (s21_size_t i = 0; i < n; i++) sum += (str[n - i - 1] - 48) * pow(10, i);
  return sum;
}  // конец s21_atoi

char *s21_ftoa(long double value, char *str, s21_size_t accuracy, int f) {
  char *p;  // указатель на начало дробной части
  char *digit;  // указатель на первую ненулевую цифру в строк. предст-ии числа
  s21_size_t n = (f ? 0 : accuracy);  // количество цифр после точки
  value = fabsl(value);
  do {
    long double a, b;  // целая и дробная части числа
    // округлить число до n знаков после точки и выделить дробн. и цел. части
    b = modfl(roundl(value * pow(10, n)) / pow(10, n), &a);
    // s21_itoa(llroundl(a), str, 10);  // преобразовать в строку целую часть
    s21_itoa(llroundl(a), str, 10);  // преобразовать в строку целую часть
    p = str + s21_strlen(str);  // найти начало дробной части в строке
    // преобразовать в строку дробную часть
    if (n > 0) s21_itoa(llroundl(b * pow(10, n)), p, 10);
    // добавить в дробную часть недостающие нули
    strchrncat(p, '0', n++ - s21_strlen(p), 0);
    if (f && value == 0 && n == accuracy) break;
    digit = s21_strpbrk(str, "123456789");
    // округлять число до количества значащих цифр
  } while (f && (!digit || s21_strlen(digit) < accuracy));
  // вставить точку перед дробной частью
  strchrncat(p, '.', (--n > 0), 0);
  return str;
}  // конец s21_ftoa

char *flt_to_str(va_list *args, char *str, s21_FORMAT *cs) {
  long double arg = 0;  // значение вещественного аргумента
  // int ll = (cs->length && cs->precision < 5 && !cs->width) ? 1 : 0;
  int ll = 0;

  if (cs->length == 0) arg = (double)va_arg(*args, double);
  if (cs->length == 'h') arg = (float)va_arg(*args, double);
  if (cs->length == 'l') arg = (double)va_arg(*args, double);
  if (cs->length == 'L') arg = (long double)va_arg(*args, long double);
  long double mantissa = arg;  // значение мантиссы аргумента
  int n = 0;  // порядок степени аргумента в экспоненциальной нотации
  // вычислить мантиссу и порядок степени аргумента

  long double left = 0, right = 0;  // long double
  int pos = 0;
  char result[256] = {0};
  right = modfl(mantissa, &left);  // modfl
  // if ((cs->length == 'L') && (left == 9999.000000) && cs->precision <= 4)
  //   ll = 1;
  char text[999] = {0};
  s21_itoa(left, text, 10);
  if (((text[0] - '0') * pow(10, 0) == 9) && (text[1] - '0') * pow(10, 0) > 4)
    ll = 1;

  if (s21_strchr("eEgG", cs->specifier)) {
    while (fabsl(mantissa) >= (ll ? 1 : 10)) {
      mantissa /= 10;
      n++;
    }
    while (0 < fabsl(mantissa) && fabsl(mantissa) < (ll ? 0 : 1)) {
      mantissa *= 10;
      n--;
    }
  }

  int f_exp = (n < -4 || n >= cs->precision);  // признак эксп. нотации для "gG"
  if (0 == arg || (1 <= arg && arg <= 9)) f_exp = 0;
  // преобразовать модуль вещественного числа или мантиссы аргумента в строку
  if (s21_strchr("f", cs->specifier)) s21_ftoa(arg, str, cs->precision, 0);
  if (s21_strchr("eE", cs->specifier))
    s21_ftoa(mantissa, str, cs->precision, 0);
  if (s21_strchr("gG", cs->specifier))
    s21_ftoa(f_exp ? mantissa : arg, str, cs->precision, 1);
  // подавить вывод нулей и точки в конце дробной части для "gG"
  if (s21_strchr("gG", cs->specifier) && !cs->flag_sharp) {
    char *dot = s21_strchr(str, '.');
    for (char *p;
         dot && (p = s21_strrchr(dot, '0')) && !s21_strpbrk(p, "123456789");)
      *p = '\0';
    if (dot && !s21_strpbrk(dot, "0123456789")) *dot = '\0';
  }
  int k = 0;  // количество знаков в итоговом префиксе +/-, ' '
  // вывести "+", " " или "-" перед положит. или отриц. значениями
  if (arg < 0) strchrncat(str, '-', (++k, 1), 0);
  if (cs->flag_plus && arg >= 0) strchrncat(str, '+', (++k, 1), 0);
  if (cs->flag_space && !cs->flag_plus && arg >= 0)
    strchrncat(str, ' ', (++k, 1), 0);
  // добавить "." при нулевой точности
  if (cs->flag_sharp && cs->precision == 0) s21_strcat(str, ".");
  // добавить суффикс эксп. нотации
  if (s21_strchr("eE", cs->specifier) ||
      (s21_strchr("gG", cs->specifier) && f_exp)) {
    strchrncat(str, (isupper(cs->specifier) ? 'E' : 'e'), 1, 1);
    strchrncat(str, (n >= 0 ? '+' : '-'), 1, 1);
    strchrncat(str, '0', abs(n) <= 9, 1);
    s21_itoa(n, str + s21_strlen(str), 10);
  }
  // дополнить пробелами или нулями (вместо пробелов) до заданной ширины
  // и выровнять вывод по левому или правому краю
  char ch = (cs->flag_zero && !cs->flag_minus ? '0' : ' ');
  strchrncat(str + (ch == '0' ? k : 0), ch, cs->width - s21_strlen(str),
             cs->flag_minus);
  return str;
}  // конец flt_to_str

int s21_sprintf(char *str, const char *format, ...) {
  va_list input = {0};
  va_start(input, format);
  s21_FORMAT parameters = {0};
  s21_size_t i = 0;
  s21_size_t lol = 0;
  s21_size_t len = 0;
  str = s21_strcpy(str, "");
  while (*format) {
    parameters.precision = -1;
    parameters.flag_plus = parameters.flag_minus = parameters.flag_space =
        parameters.flag_zero = parameters.flag_sharp = parameters.width =
            parameters.point = parameters.specifier = parameters.length = 0;
    switch (*format) {
      case '%':
        format++;
        if (is_flag(*format)) {
          while (is_flag(*format)) {
            s21_check_flag(&parameters, format++);
          }
        }
        if (is_digit(*format) || *format == '*') {
          format = s21_str_to_int(&parameters.width, format++, &input);
          if (parameters.width < 0) {
            parameters.flag_minus = 1;
            parameters.width *= -1;
          }
        }
        if (*format == '.') {
          parameters.point = 1;
          parameters.precision = 0;
          format++;
          if (is_digit(*format) || *format == '*') {
            format = s21_str_to_int(&parameters.precision, format++, &input);
          } else {
            parameters.precision = -1;
          }
        }
        if (is_length(*format)) {
          while (is_length(*format)) {
            parameters.length = *(format++);
          }
        }
        if (is_specifier(*format)) {
          parameters.specifier = *(format++);
        }
        s21_default_precision(&parameters);
        str = s21_check_specifier(str, &i, &input, &parameters);
        if (parameters.stop == 1) {
          lol = i;
          parameters.stop = 0;
          parameters.returnN = 1;
        }

        break;
      default:
        str[i++] = *(format++);
        str[i] = '\0';
        break;
    }
  }
  va_end(input);
  if (parameters.returnN == 1) {
    len = (int)s21_strlen(str) + parameters.nulls;
    str[lol] = '\0';
  } else
    len = (int)s21_strlen(str);
  // if (parameters.stop == 1) return (int)s21_strlen(str) + 1;
  return len;
  // return (int)s21_strlen(str) + parameters.nulls;
}

void s21_default_precision(s21_FORMAT *parameters) {
  if (parameters->point && parameters->precision <= 0) {
    parameters->precision = 0;
  }
  if (!parameters->point && s21_strchr("eEfgG", parameters->specifier)) {
    parameters->precision = 6;
  }
}
void s21_check_flag(s21_FORMAT *parameters, const char *format) {
  switch (*format) {
    case '-':
      parameters->flag_minus = 1;
      break;
    case '+':
      parameters->flag_plus = 1;
      break;
    case ' ':
      parameters->flag_space = 1;
      break;
    case '#':
      parameters->flag_sharp = 1;
      break;
    case '0':
      parameters->flag_zero = 1;
      break;
  }
}

char *s21_check_specifier(char *str, s21_size_t *str_len, va_list *input,
                          s21_FORMAT *parameters) {
  char *tmp_str = s21_NULL;
  tmp_str = calloc(1024, sizeof(char));
  if (tmp_str != s21_NULL) {
    switch (parameters->specifier) {
      case 'c':
        s21_spec_c(tmp_str, input, parameters);
        break;
      case 'd':
      case 'i':
        s21_spec_di(tmp_str, input, parameters);
        break;
      case 'e':
      case 'E':
        // s21_spec_eE(tmp_str, input, parameters);
        flt_to_str(input, tmp_str, parameters);
        break;
      case 'f':
        // s21_spec_f(tmp_str, input, parameters);
        flt_to_str(input, tmp_str, parameters);
        break;
      case 'g':
      case 'G':
        // s21_spec_gG(tmp_str, input, parameters);
        flt_to_str(input, tmp_str, parameters);
        break;
      case 'n':
        s21_spec_n(str_len, input);
        break;
      case 'o':
        s21_spec_o(tmp_str, input, parameters);
        break;
      case 'p':
        s21_spec_p(tmp_str, input, parameters);
        break;
      case 's':
        // s21_spec_s(tmp_str, input, parameters);
        s22str_to_str(input, tmp_str, parameters);
        break;
      case 'u':
        s21_spec_u(tmp_str, input, parameters);
        break;
      case 'x':
      case 'X':
        s21_spec_xX(tmp_str, input, parameters);
        break;
      case '%':
        s21_spec_percentage(tmp_str, parameters);
        break;
    }
    s21_strcat(str, tmp_str);
    free(tmp_str);
    *str_len = s21_strlen(str);
  }
  return str;
}

char *s21_str_to_int(int *number, const char *format, va_list *input) {
  char wth[128] = {0};
  char *num = NULL;
  int count = 0;
  while (is_digit(*format)) {
    wth[count++] = *(format)++;
  }
  num = wth;
  while (count > 0) {
    *number += ((*(num)++) - '0') * pow(10, --count);
  }
  if (*format == '*') {
    *number = va_arg(*input, int);
    format++;
  }
  return (char *)format;
}

void s21_int_to_str(s21_FORMAT *parameters, char *str, long double number) {
  char *point = s21_NULL;
  int flag = 1;
  parameters->sign = number < 0 ? -1 : 1;
  number *= parameters->sign;
  point = str;
  if (parameters->specifier != 'n') {
    while (flag) {
      int digit = 0;
      if (number >= 10) {
        digit = (int)fmod(number, 10);
        number = (number - digit) / 10;
      } else {
        digit = roundl(number);  //(int)number   fff
        // digit = (int)number;  //(int)number
        flag = 0;
      }
      *(point++) = (char)(digit + '0');
    }
    if (s21_strchr("dioxX", parameters->specifier) &&
        parameters->precision == 0 && number == 0 && parameters->point)
      str[0] = '\0';
    *point = '\0';
    s21_reverse_writing(str);
  }
}

void s21_ul_to_str(char *str, unsigned long long number,
                   s21_FORMAT *parameters) {
  char *point = s21_NULL;
  int flag = 1;

  point = str;

  while (flag) {
    int digit = 0;
    if (number >= 10) {
      digit = (int)fmod(number, 10);
      number = (number - digit) / 10;
    } else {
      digit = (int)number;
      flag = 0;
    }
    *(point++) = (char)(digit + '0');
  }
  if (parameters->precision == 0 && number == 0 && parameters->point)
    str[0] = '\0';
  *point = '\0';
  s21_reverse_writing(str);
}

void s21_ol_to_str(char *str, unsigned long long number, int prefix) {
  char *point = s21_NULL;
  int flag = 1;
  int nulevoi = 0;

  point = str;

  while (flag) {
    unsigned int digit;
    if (number > 7) {
      digit = number % 8;
      number = (number - digit) / 8;
    } else {
      digit = number;
      flag = 0;
    }
    *(point++) = (char)(digit + '0');
  }
  if (str[0] == '0' && str[1] == '\0') {
    nulevoi = 1;
  }

  if (prefix && !nulevoi) *(point++) = '0';  // prefix == 1 && nulevoy != 1
  *point = '\0';
  s21_reverse_writing(str);
}

void s21_hl_to_str(char *str, unsigned long long number,
                   s21_FORMAT *parameters) {
  char *point = s21_NULL;
  int flag = 1, shift = 0;
  point = str;
  shift = parameters->specifier == 'x' ? 32 : 0;

  while (flag) {
    unsigned int digit;
    if (number > 15) {
      digit = number % 16;
      number = (number - digit) / 16;
    } else {
      digit = number;
      flag = 0;
    }
    if (digit < 10)
      *(point++) = (char)(digit + '0');
    else
      *(point++) = (char)(digit + 'A' + shift - 10);
  }
  *point = '\0';
  if (number == 0) parameters->flag_sharp = 0;
  s21_reverse_writing(str);
}

void s21_float_to_str(char *str, long double number, s21_FORMAT *parameters) {
  parameters->sign = number < 0 ? -1 : 1;
  number *= parameters->sign;
  int local_sign = parameters->sign;
  if (parameters->precision <= 0) {
    s21_int_to_str(parameters, str, roundl(number));
  } else {
    long double left = 0, right = 0;  // long double
    int pos = 0;
    char result[256] = {0};
    right = modfl(number, &left);  // modfl
    if ((parameters->length == 'L') && (left == 9999.000000) &&
        parameters->precision <= 4) {
      number = roundl(number);
      right = modfl(number, &left);
    }

    if (s21_strchr("gG", parameters->specifier) && parameters->precision == 1 &&
        right >= 0.5) {
      left++;
    }
    s21_int_to_str(parameters, str, roundl(left));
    s21_strcat(str, ".");
    int local_precision = parameters->precision;
    if (s21_strchr("gG", parameters->specifier)) {
      if (left >= 1) {
        local_precision = parameters->precision - (int)s21_strlen(str) + 1 +
                          ((int)roundl(number) == 0);
      } else if (right * pow(10, local_precision) < 1) {
        local_precision = 4;
      }
    }
    for (int i = 0; i < local_precision; i++) right *= 10;
    s21_int_to_str(parameters, result, roundl(right));
    pos = (int)s21_strlen(result);
    for (int i = 0; pos < local_precision; i++, pos++) {
      s21_move_string(result);
      result[i] = '0';
    }
    if (s21_strchr("gG", parameters->specifier) && (int)roundl(number) != 0) {
      result[parameters->precision - (int)s21_strlen(str) + 1] = '\0';
    }
    s21_strcat(str, result);
  }
  parameters->sign = local_sign;
}

void s21_gG_to_str(char *str, long double number, s21_FORMAT *parameters) {
  parameters->sign = number < 0 ? -1 : 1;
  number *= parameters->sign;
  int local_sign = parameters->sign;

  if ((number / pow(10, parameters->precision)) > 1 ||
      ((number * (pow(10, 4))) < 1 && (number * (pow(10, 21))) >= 1)) {
    if (number < 1 && parameters->precision <= 0) {
      parameters->precision = 4;
    } else {
      --parameters->precision;
    }
    s21_eE_to_str(str, number, parameters);
  } else {
    parameters->precision > 0 ? 1 : ++parameters->precision;
    s21_float_to_str(str, number, parameters);
    s21_delete_zero(parameters, str);
  }
  parameters->sign = local_sign;
}

void s21_delete_zero(s21_FORMAT *parameters, char *str) {
  int pos = (int)s21_strlen(str) - 1;
  if (pos > 0) {
    while (str[pos] == '0') {
      if (!parameters->flag_sharp ||
          (parameters->flag_sharp && pos > parameters->precision)) {
        str[pos] = '\0';
        pos -= 1;
      }
    }
    if (!parameters->flag_sharp && str[pos] == '.') {
      str[pos] = '\0';
    }
  }
}

void s21_eE_to_str(char *str, long double number, s21_FORMAT *parameters) {
  parameters->sign = number < 0 ? -1 : 1;
  int notation = 0, local_sign = parameters->sign;
  number *= parameters->sign;
  if (number < 1) {
    while (number < 1 && notation-- > -42) {
      number *= 10;
    }
    number < 1 ? (notation = number = 0) : 1;

    if (s21_compare_round(parameters, number)) {
      if (parameters->precision >= 0) {
        --parameters->precision;
      }
      notation++;
    }
    if (s21_strchr("gG", parameters->specifier))
      s21_delete_zero(parameters, str);
    s21_make_mantissa(parameters, &str[s21_strlen(str)], notation);
  } else {
    while (number >= 10) {
      notation++;
      number /= 10;
    }
    if (s21_compare_round(parameters, number)) {
      if (parameters->precision >= 0) {
        --parameters->precision;
      }
      notation++;
    }
  }

  if (parameters->precision == -1) parameters->precision = 0;  /////
  number = roundl(number *
                  pow(10, parameters->precision));  /// parameters->precision
  s21_int_to_str(parameters, str, number);
  if (parameters->precision > 0) {
    s21_move_string(&str[1]);
    str[1] = '.';
    for (int i = (int)s21_strlen(str); i < 2 + parameters->precision; i++) {
      str[i] = '0';
    }
  }
  if (s21_strchr("gG", parameters->specifier)) s21_delete_zero(parameters, str);
  s21_make_mantissa(parameters, &str[s21_strlen(str)], notation);
  parameters->sign = local_sign;
}

// Если округляется до следующего разряда, необходимо увеличить степень
// и поделить число на 10
// В этом случае возвращаем True
int s21_compare_round(s21_FORMAT *parameters, long double number) {
  char compare_str[128] = {'\0'};
  int length = 0, result = 0;
  number *= pow(10, parameters->precision);
  s21_int_to_str(parameters, compare_str, number);
  length = s21_strlen(compare_str);
  number = roundl(number);
  s21_int_to_str(parameters, compare_str, number);
  if (length < (int)s21_strlen(compare_str)) {
    result = 1;
  }
  return result;
}

void s21_make_mantissa(s21_FORMAT *parameters, char *mantice, int notation) {
  int start_pos = 0;
  if (parameters->flag_sharp && parameters->precision <= 0) {
    mantice[start_pos++] = '.';
  }
  mantice[start_pos++] = s21_strchr("eg", parameters->specifier) ? 'e' : 'E';
  mantice[start_pos++] = (notation >= 0) ? '+' : '-';
  if (abs(notation) < 10) {
    mantice[start_pos++] = '0';
    s21_int_to_str(parameters, &mantice[start_pos], abs(notation));
  } else {
    s21_int_to_str(parameters, &mantice[start_pos], abs(notation));
  }
}

void s21_wch_to_str(char *str, wchar_t *wstr, s21_size_t length) {
  s21_size_t str_len = s21_strlen(str);
  int cnt = length;
  char *p = str + str_len;

  while (*wstr != '\0' && (cnt-- > 0 || length == 0)) {
    *(p++) = (char)*(wstr++);
  }
  *p = '\0';
}

/* Функция сдвига строки на 1 позицию вправо */
void s21_move_string(char *str) {
  int pos = (int)s21_strlen(str);
  str[pos + 1] = '\0';
  while (pos > 0) {
    str[pos] = str[pos - 1];
    pos--;
  }
}

void s21_move_string_left(char *str) {
  int pos = (int)s21_strlen(str);
  int i = 0;
  str[pos + 1] = '\0';
  while (pos > 0) {
    str[i] = str[i + 1];
    i++;
    pos--;
  }
}

void s21_make_string_flags(s21_FORMAT *parameters, char *str) {
  int pos = 0;
  if (!s21_strchr("cuo\%", parameters->specifier)) {
    if (parameters->specifier != 'x' && parameters->specifier != 'X') {
      if (parameters->sign < 0) {
        s21_move_string(str);
        str[pos++] = '-';
        parameters->sign = 0;
      }
      if (parameters->flag_plus && parameters->sign != 0) {
        s21_move_string(str);
        str[pos] = (parameters->sign > 0) ? '+' : '-';
        parameters->sign = 0;
      }
      if (parameters->flag_space && parameters->sign != 0) {
        s21_move_string(str);
        str[pos] = (parameters->sign > 0) ? ' ' : '-';
        parameters->sign = 0;
      }
    }
    if (parameters->flag_sharp) {
      s21_sharp_flag(parameters, str);
    }
  }
  if (parameters->flag_minus) {
    pos = (int)s21_strlen(str);
    while (pos < parameters->width) {
      str[pos++] = ' ';
    }
  }
}

/* Функция обработки флага '#' */
void s21_sharp_flag(s21_FORMAT *parameters, char *str) {
  int pos = 0;
  if (s21_strchr("oxX", parameters->specifier)) {
    if (parameters->specifier == 'x' || parameters->specifier == 'X') {
      s21_move_string(str);
    }
    parameters->specifier == 'o' ? 1 : s21_move_string(str);
    str[pos++] = '0';
    if (parameters->specifier != 'o')
      str[pos] = parameters->specifier == 'x' ? 'x' : 'X';
  }
  if (s21_strchr("eEf", parameters->specifier)) {
    if (!s21_strchr(str, '.')) {
      str[s21_strlen(str)] = '.';
    }
  }
}

/* Функция обработки ширины форматной строки */
void s21_make_string_width(s21_FORMAT *parameters, char *str) {
  int pos_str = (int)s21_strlen(str);
  int minus = 0;
  int nulevoi = 0;
  if (str[0] == '0' && str[1] == '\0') {
    nulevoi = 1;
  }

  char znak;
  int local_width = 0;
  local_width = parameters->width;

  // if (str[0] == '-' && parameters->flag_zero) {
  //   minus = 1;
  //   s21_move_string_left(str);
  //   parameters->width += 1;
  // }
  if (parameters->width > pos_str) {
    if ((((str[0] == '-') || (str[0] == '+') || str[0] == ' ') &&
         (parameters->flag_zero) && ((parameters->specifier != 'c'))) ||
        ((parameters->flag_zero) && (parameters->flag_plus) &&
         ((parameters->specifier != 'c')))) {
      // znak = str[0] == '-' ? '-' : '+';
      switch (str[0]) {
        case '-':
          znak = '-';
          minus = 1;
          break;
        case '+':
          znak = '+';
          minus = 1;
          break;
        case ' ':
          znak = ' ';
          minus = 1;
          break;
      }
      // minus = 1;
      s21_move_string_left(str);
      parameters->width += 1;
    }

    str[parameters->width + 1] = '\0';
    while (pos_str >= 0) {
      str[parameters->width--] = str[pos_str--];
    }
    for (pos_str = 0; pos_str <= parameters->width; pos_str++) {
      if (s21_strchr("diopuxX", parameters->specifier)) {
        str[pos_str] =
            (parameters->flag_zero && !parameters->point) ? '0' : ' ';
      }
      if (s21_strchr("dfeEgGcs", parameters->specifier) && minus == 1) {
        str[pos_str] = (parameters->flag_zero) ? '0' : ' ';
        if (pos_str == parameters->width) {
          str[0] = znak;
        }
      }

      if (s21_strchr("feEgGcs", parameters->specifier)) {
        str[pos_str] = (parameters->flag_zero) ? '0' : ' ';
      }
    }  //%.X
  }
  if (parameters->point && parameters->precision == 0 && local_width >= 0 &&
      nulevoi == 1) {
    if (local_width == 0) {
      str[0] = '\0';
    } else
      s21_memset(str, ' ', local_width);
  }
}

/* Функция обработки точности форматной строки */
void s21_make_string_precision(s21_FORMAT *parameters, char *str) {
  char *string_with_precision = s21_NULL;
  string_with_precision = calloc(1024, sizeof(char));
  int pos = 0;
  int point = 0;
  // int pop = 1;
  if ((parameters->specifier == 'p' && !parameters->flag_minus &&
       ((parameters->width >= 0 && parameters->precision >= 9) ||
        (parameters->width >= 0 && parameters->precision >= 0 &&
         (s21_strcmp(str, "0x0") == 0))))) {
    ;
    /* code */

    point = 2;
    s21_move_string_left(str);
    s21_move_string_left(str);
  }
  if (string_with_precision != s21_NULL) {
    if (s21_strchr("pdiouxX", parameters->specifier)) {
      while (pos < parameters->precision - (int)s21_strlen(str) + point) {
        string_with_precision[pos++] = '0';
      }
      for (int i = 0; str[i] != '\0'; i++) {
        string_with_precision[pos++] = str[i];
      }
      s21_strcpy(str, string_with_precision);
      if (point > 0) {
        str[0] = '0';
        str[1] = 'x';
      }
    }
    free(string_with_precision);
  }
}
// s21 === *=   0000000000000x1056bb6d9=* = 25=
//  man === *=0x00000000000000100cbb6d9=*

/* Функция обратной записи в строку */
void s21_reverse_writing(char *str) {
  char c = 0;
  s21_size_t length = s21_strlen(str);
  for (s21_size_t i = 0; i < length / 2; i++) {
    c = str[i];
    str[i] = str[length - 1 - i];
    str[length - 1 - i] = c;
  }
}

/* Символ */
void s21_spec_c(char *str, va_list *input, s21_FORMAT *parameters) {
  wchar_t wchar = 0;
  wchar_t wchar_s[10] = {0};
  switch (parameters->length) {
    case 'l':
      wchar = va_arg(*input, wchar_t);
      wchar_s[0] = wchar;
      s21_wch_to_str(str, wchar_s, 0);
      break;
    default:
      str[0] = va_arg(*input, int);
      if (str[0] == '\0') {
        parameters->nulls += 1;
        if (parameters->returnN != 1) {
          parameters->stop = 1;
        }
      }

      str[1] = '\0';
      break;
  }
  s21_make_string_flags(parameters, str);
  s21_make_string_width(parameters, str);
}

/* Знаковое десятичное целое число */
void s21_spec_di(char *str, va_list *input, s21_FORMAT *parameters) {
  long int ld_number = 0;
  short int sd_number = 0;
  int number = 0;

  switch (parameters->length) {
    case 'h':
      sd_number = (short int)va_arg(*input, int);
      s21_int_to_str(parameters, str, sd_number);
      break;
    case 'l':
      ld_number = (long int)va_arg(*input, long int);
      s21_int_to_str(parameters, str, ld_number);
      break;
    default:
      number = va_arg(*input, int);
      s21_int_to_str(parameters, str, number);
      break;
  }
  s21_make_string_precision(parameters, str);
  s21_make_string_flags(parameters, str);
  s21_make_string_width(parameters, str);
}

/* Научная нотация (мантисса/экспонента) с использованием символа e/E (вывод
 * чисел должен совпадать с точностью до e-6) */
void s21_spec_eE(char *str, va_list *input, s21_FORMAT *parameters) {
  long double l_number = 0;
  double number = 0;
  switch (parameters->length) {
    case 'L':
      l_number = va_arg(*input, long double);
      s21_eE_to_str(str, l_number, parameters);
      break;
    default:
      number = va_arg(*input, double);
      s21_eE_to_str(str, number, parameters);
      break;
  }
  s21_make_string_precision(parameters, str);
  s21_make_string_flags(parameters, str);
  s21_make_string_width(parameters, str);
}

void s21_spec_f(char *str, va_list *input, s21_FORMAT *parameters) {
  long double l_number = 0;
  double d_number = 0;

  switch (parameters->length) {
    case 'L':
      l_number = va_arg(*input, long double);
      s21_float_to_str(str, l_number, parameters);
      break;
    case 'l':
    default:
      d_number = va_arg(*input, double);
      s21_float_to_str(str, d_number, parameters);
      break;
  }
  s21_make_string_flags(parameters, str);
  s21_make_string_width(parameters, str);
}

/* Использует кратчайший из представлений десятичного числа */
void s21_spec_gG(char *str, va_list *input, s21_FORMAT *parameters) {
  long double l_number = 0;
  long double inf_number = 0;
  double number = 0;
  // long double arg = 0;
  // long double mantissa = arg;  // значение мантиссы аргумента
  // int n = 0;  // порядок степени аргумента в экспоненциальной нотации
  // int f_exp =
  //     (n < -4 || n >= parameters->precision);  // признак эксп. нотации для
  //     "gG"
  // if (0 == arg || (1 <= arg && arg <= 9)) f_exp = 0;

  inf_number = va_arg(*input, long double);
  if (inf_number != INFINITY) {
    switch (parameters->length) {
      case 'L':
        l_number = va_arg(*input, long double);
        // s21_gG_to_str(str, l_number, parameters);
        s21_gG_to_str(str, l_number, parameters);
        break;
      case 'l':
      default:
        number = va_arg(*input, double);
        s21_gG_to_str(str, number, parameters);
        break;
    }
  } else
    s21_strcat(str, "inf");
  s21_make_string_precision(parameters, str);
  s21_make_string_flags(parameters, str);
  s21_make_string_width(parameters, str);
}

/* Беззнаковое восьмеричное число */
void s21_spec_o(char *str, va_list *input, s21_FORMAT *parameters) {
  unsigned long int ld_number = 0;
  unsigned short int sd_number = 0;
  unsigned int number = 0;

  switch (parameters->length) {
    case 'h':
      sd_number = (unsigned short int)va_arg(*input, unsigned int);
      s21_ol_to_str(str, sd_number, parameters->flag_sharp);
      break;
    case 'l':
      ld_number = (unsigned long int)va_arg(*input, unsigned long int);
      s21_ol_to_str(str, ld_number, parameters->flag_sharp);
      break;
    default:
      number = va_arg(*input, unsigned int);
      s21_ol_to_str(str, number, parameters->flag_sharp);
      break;
  }
  s21_make_string_precision(parameters, str);
  s21_make_string_flags(parameters, str);
  s21_make_string_width(parameters, str);
}

/* Строка символов */
void s21_spec_s(char *str, va_list *input, s21_FORMAT *parameters) {
  char *string = s21_NULL;
  // parameters->precision = INT_MAX;
  string = va_arg(*input, char *);
  if (parameters->precision > -1)
    s21_strncat(str, string, parameters->precision);
  else
    s21_strcat(str, string);
  s21_make_string_precision(parameters, str);
  s21_make_string_flags(parameters, str);
  s21_make_string_width(parameters, str);
}

char *s22str_to_str(va_list *args, char *str, s21_FORMAT *cs) {
  // строковое значение аргумента ограничить точностью
  if (cs->length == 0) {
    char *tmpstr = va_arg(*args, char *);
    s21_strncat(str, (tmpstr ? tmpstr : "(null)"), cs->precision);
  }
  // широкую строку аргумента преобразовать в многобайтовую строку
  // и ограничить ее точностью
  if (cs->length == 'l') {
    wchar_t *pwcs = va_arg(*args, wchar_t *);
    int n = wcstombs(str, pwcs, wcslen(pwcs) * MB_CUR_MAX);
    if (cs->precision < n) n = cs->precision;
    str[n] = '\0';
  }
  // дополнить пробелами до заданной ширины
  // и выровнять вывод по левому или правому краю
  strchrncat(str, cs->flag_zero ? '0' : ' ', cs->width - s21_strlen(str),
             cs->flag_minus);
  return str;
}  // конец str_to_str

/* Беззнаковое десятичное целое число */
void s21_spec_u(char *str, va_list *input, s21_FORMAT *parameters) {
  unsigned short us_number = 0;
  unsigned long ul_number = 0;
  unsigned int u_number = 0;

  switch (parameters->length) {
    case 'h':
      us_number = (unsigned short)va_arg(*input, unsigned int);
      s21_ul_to_str(str, us_number, parameters);
      break;
    case 'l':
      ul_number = (unsigned long)va_arg(*input, unsigned long);
      s21_ul_to_str(str, ul_number, parameters);
      break;
    default:
      u_number = (unsigned int)va_arg(*input, unsigned int);
      s21_ul_to_str(str, u_number, parameters);
      break;
  }

  s21_make_string_precision(parameters, str);
  s21_make_string_flags(parameters, str);
  s21_make_string_width(parameters, str);
}

/* Беззнаковое шестнадцатеричное целое число(X - заглавные буквы) */
void s21_spec_xX(char *str, va_list *input, s21_FORMAT *parameters) {
  unsigned long int ld_number = 0;
  unsigned short int sd_number = 0;
  unsigned int number = 0;

  switch (parameters->length) {
    case 'h':
      sd_number = (unsigned short int)va_arg(*input, unsigned int);
      s21_hl_to_str(str, sd_number, parameters);
      break;
    case 'l':
      ld_number = (unsigned long int)va_arg(*input, unsigned long int);
      s21_hl_to_str(str, ld_number, parameters);
      break;
    default:
      number = (unsigned int)va_arg(*input, unsigned int);
      s21_hl_to_str(str, number, parameters);
      break;
  }
  s21_make_string_precision(parameters, str);
  s21_make_string_flags(parameters, str);
  s21_make_string_width(parameters, str);
}

/* Адрес указателя */
void s21_spec_p(char *str, va_list *input, s21_FORMAT *parameters) {
  int width = parameters->width < 15 ? 15 : parameters->width;
  char *str_start = s21_NULL;
  str_start = str;
  int *p = va_arg(*input, int *);
  char buf[4] = "x0";
  if (p == s21_NULL) {
    s21_strcpy(buf, "0x0");
  }
  for (int *j = p, k = 0; j && k < width;
       j = (void *)(((s21_size_t)j) >> 4), k++) {
    unsigned int dig = ((s21_size_t)j) % 0x10;
    if (dig < 10)
      *(str_start++) = (char)('0' + dig);
    else
      *(str_start++) = (char)('a' + (dig - 10));
  }
  *str_start = '\0';
  s21_strcat(str, buf);
  s21_reverse_writing(str);
  s21_make_string_precision(parameters, str);
  s21_make_string_flags(parameters, str);
  s21_make_string_width(parameters, str);
}

/* Количество символов, напечатанных до появления %n */
void s21_spec_n(s21_size_t const *str_len, va_list *input) {
  int *p = va_arg(*input, int *);
  *p = (int)*str_len;
}

/* Символ % */
void s21_spec_percentage(char *str, s21_FORMAT *parameters) {
  s21_strcat(str, "%");
  s21_make_string_precision(parameters, str);
  s21_make_string_flags(parameters, str);
  s21_make_string_width(parameters, str);
}
#define test
#ifdef test
int main(int argc, char const *argv[]) {
  int h = 0;
  int m = 0;

  char s1[] = " wtf ";
  char *s3 = " 1";
  char *s4 = " wtf ";
  char *s2 = s21_trim(s1, s3);

  printf("s21 === *%s* = %d=\n", s2, h);
  printf("man === *%s* = %d=\n", s2, m);

  return 0;
}
#endif

//*1 Test  Test  Test  Test c* man === *=       inf=* = 12=
// s21 === *test: -9223372036854775807.0!  test: -9223372036854775807.00!  test:
// -9223372036854775807.000!* = 92= man === *test: -9325781235683690496.0! test:
// -9325781235683690496.00!  test: -9325781235683690496.000!* = 92=