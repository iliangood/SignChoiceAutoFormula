#include "fractions.h"
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max(a, b) ((a > b) ? (a) : (b))
#define min(a, b) ((a < b) ? (a) : (b))

bool isCorrect(number num) { return num.numerator; }

long long llpow(long long num, long long power) {
  if (power < 0)
    return 0;
  if (power == 0)
    return 1;
  bool sign = (power % 2) && num < 0;
  long long res = 1;
  while (power >= 1) {
    if (power % 2) {
      power--;
      res *= num;
    } else {
      power /= 2;
      num *= num;
    }
  }
  return res;
}

long long gcd(long long a, long long b) {
  if (a == 0 || b == 0)
    return 1;
  a = llabs(a);
  b = llabs(b);
  while (max(a, b) % min(a, b)) {
    if (a > b) {
      a = a % b;
    } else {
      b = b % a;
    }
  }
  return min(a, b);
}

number simplify(number num) {
  int greatest_common_diviser = gcd(num.numerator, num.denominator);
  num.numerator /= greatest_common_diviser;
  num.denominator /= greatest_common_diviser;
  return num;
}

number signRecovery(number num) {
  num.numerator *= (num.denominator < 0) ? -1 : 1;
  num.denominator = llabs(num.denominator);
  return num;
}

number fractionRecovery(number num) { return simplify(signRecovery(num)); }

number addn(number a, number b) {
  number res;
  res.numerator = a.numerator * b.denominator + b.numerator * a.denominator;
  res.denominator = a.denominator * b.denominator;
  res = fractionRecovery(res);
  return res;
}

number subn(number a, number b) {
  number res;
  res.numerator = a.numerator * b.denominator - b.numerator * a.denominator;
  res.denominator = a.denominator * b.denominator;
  res = fractionRecovery(res);
  return res;
}

number muln(number a, number b) {
  number res;
  res.numerator = a.numerator * b.numerator;
  res.denominator = a.denominator * b.denominator;
  res = fractionRecovery(res);
  return res;
}

number divn(number a, number b) {
  number res;
  res.numerator = a.numerator * b.denominator;
  res.denominator = a.denominator * b.numerator;
  res = fractionRecovery(res);
  return res;
}

number absn(number num) {
  num.numerator = llabs(num.numerator);
  num.denominator = llabs(num.denominator);
  return num;
}

bool hasOtherFactors(long long n) {
  n = llabs(n);
  while (n % 2 == 0)
    n /= 2;
  while (n % 5 == 0)
    n /= 5;
  return n != 1;
}

number iton(long long num) {
  number res;
  res.numerator = num;
  res.denominator = 1;
  return res;
}

bool isNegative(number num) {
  num = fractionRecovery(num);
  return num.numerator < 0;
}

int cmpn(number a, number b) {
  a.numerator *= b.denominator;
  b.numerator *= a.denominator;
  return (a.numerator > b.numerator) - (a.numerator < b.numerator);
}

char *ntoda(number num) {
  num = fractionRecovery(num);
  if (num.denominator == 0) {
    return NULL;
  }

  if (hasOtherFactors(num.denominator)) {
    return NULL;
  }

  int is_negative = num.numerator < 0;
  num.numerator = llabs(num.numerator);

  int twos = 0, fives = 0;
  long long den = num.denominator;
  while (den % 2 == 0) {
    den /= 2;
    twos++;
  }
  while (den % 5 == 0) {
    den /= 5;
    fives++;
  }
  int decimal_places = max(twos, fives);

  int max_digits = 30;
  int buffer_size = max_digits + 1;
  if (decimal_places > 0) {
    buffer_size += 1 + decimal_places;
  }

  char *result = (char *)malloc(buffer_size * sizeof(char));
  if (result == NULL) {
    return NULL;
  }

  if (decimal_places == 0) {
    snprintf(result, buffer_size, "%s%lld", is_negative ? "-" : "",
             num.numerator / num.denominator);
  } else {
    char format[20];
    snprintf(format, 20, "%%s%%lld.%%0%dlld", decimal_places);
    long long ten_pow = llpow(10, decimal_places);
    // for (int i = 0; i < decimal_places; i++) ten_pow *= 10;
    long long decimal_part =
        (num.numerator % num.denominator) *
        ((twos > fives) ? llpow(5, twos - fives) : llpow(2, fives - twos));
    long long integer_part = num.numerator / num.denominator;
    snprintf(result, buffer_size, format, is_negative ? "-" : "", integer_part,
             decimal_part);
  }

  size_t actual_size = strlen(result) + 1;
  char *resized_result = (char *)realloc(result, actual_size * sizeof(char));
  if (resized_result == NULL) {
    return result;
  }
  return resized_result;
}

char *ntosfa(number num) {
  num = fractionRecovery(num);
  char *str = malloc(60);
  if (str == NULL)
    return NULL;
  snprintf(str, 60, "%lld/%lld", num.numerator, num.denominator);
  char *res = realloc(str, strlen(str) + 1);
  if (res == NULL)
    return str;
  return res;
}

char *ntofwi(number num) {
  num = fractionRecovery(num);
  char *str = malloc(60);
  if (str == NULL)
    return NULL;
  long long integer_part = num.numerator / num.denominator;
  snprintf(str, 60, "%lld %lld/%lld", integer_part,
           num.numerator % num.denominator, num.denominator);
  char *res = realloc(str, strlen(str) + 1);
  if (res == NULL)
    return str;
  return res;
}

char *ntofa(number num) {
  num = fractionRecovery(num);
  long long integer_part = num.numerator / num.denominator;
  if (integer_part)
    return ntofwi(num);
  return ntosfa(num);
}

char *ntoa(number num) {
  num = fractionRecovery(num);
  char *res = ntoda(num);
  if (res != NULL)
    return res;
  return ntofa(num);
}

// number aton(char *str) {
//   if (str == NULL)
//     return (number){0, 0};
//   long long integer_part, numerator, denominator;
//   char next_char = 0;
//   int test = sscanf(str, "%lld%c", &integer_part, &next_char);
//   if (test == 1 || test == 2)
//     if (next_char == 0 || isspace(next_char))
//       return (number){integer_part, 1};
//   if (sscanf(str, "%lld+%lld/%lld", &integer_part, &numerator, &denominator)
//   ==
//       3)
//     return fractionRecovery(
//         (number){integer_part * denominator + numerator, denominator});
//   if (sscanf(str, "%lld/%lld", &numerator, &denominator) == 2)
//     return fractionRecovery((number){numerator, denominator});
//   char *dec = malloc(30);
//   if (dec == NULL)
//     return (number){0, 0};
//   if (sscanf(str, "%lld.%s", &integer_part, dec) == 2) {
//     denominator = llpow(10, strlen(dec));
//     numerator = atoi(dec) + integer_part * denominator;
//     free(dec);
//     return fractionRecovery((number){numerator, denominator});
//   }
//   free(dec);
//   return (number){0, 0};
// }

// static bool parse_u64_str(const char *s, size_t len, unsigned long long *out)
// {
//   if (len == 0)
//     return false;
//   unsigned long long v = 0;
//   for (size_t i = 0; i < len; ++i) {
//     char c = s[i];
//     if (c < '0' || c > '9')
//       return false;
//     unsigned digit = (unsigned)(c - '0');
//     if (v > ULLONG_MAX / 10)
//       return false; /* переполнение */
//     v *= 10;
//     if (v > ULLONG_MAX - digit)
//       return false;
//     v += digit;
//   }
//   *out = v;
//   return true;
// }
//
// /* Возвращает 10^n в ull, или false при переполнении */
// static bool pow10_u64(size_t n, unsigned long long *out) {
//   unsigned long long v = 1;
//   for (size_t i = 0; i < n; ++i) {
//     if (v > ULLONG_MAX / 10)
//       return false;
//     v *= 10;
//   }
//   *out = v;
//   return true;
// }
//
// number aton(const char *str) {
//   if (str == NULL)
//     return (number){0, 0};
//   size_t len = strlen(str);
//   if (len == 0)
//     return (number){0, 0};
//
//   size_t i = 0;
//   bool is_negative = false;
//
//   /* знак в начале (или '+' или '-') */
//   if (str[0] == '-' || str[0] == '+') {
//     is_negative = (str[0] == '-');
//     i = 1;
//     if (len == 1)
//       return (number){0, 0}; /* только знак — ошибка */
//   }
//
//   /* Случай: .<digits> (десятичная часть без целой) */
//   if (i < len && str[i] == '.') {
//     ++i;
//     size_t start_dec = i;
//     while (i < len && (unsigned char)str[i] >= '0' &&
//            (unsigned char)str[i] <= '9')
//       ++i;
//     size_t end_dec = i;
//     if (start_dec == end_dec)
//       return (number){0, 0}; /* пустая десятичная часть */
//     if (i != len)
//       return (number){0, 0}; /* дальше ничего быть не должно */
//     unsigned long long dec_part;
//     if (!parse_u64_str(str + start_dec, end_dec - start_dec, &dec_part))
//       return (number){0, 0};
//     unsigned long long denom;
//     if (!pow10_u64(end_dec - start_dec, &denom))
//       return (number){0, 0}; /* переполнение */
//     long long numer_signed;
//     if (dec_part > (unsigned long long)LLONG_MAX)
//       return (number){0, 0}; /* выходит за i64 */
//     numer_signed = (long long)dec_part;
//     if (is_negative)
//       numer_signed = -numer_signed;
//     return fractionRecovery((number){numer_signed, denom});
//   }
//
//   /* Парсим первую (целую) последовательность цифр */
//   size_t start_first = i;
//   while (i < len && (unsigned char)str[i] >= '0' &&
//          (unsigned char)str[i] <= '9')
//     ++i;
//   size_t end_first = i;
//   if (end_first == start_first)
//     return (number){0, 0}; /* нет цифр */
//
//   unsigned long long first_num;
//   if (!parse_u64_str(str + start_first, end_first - start_first, &first_num))
//     return (number){0, 0};
//
//   /* Если строка кончилась — целое число */
//   if (end_first == len) {
//     long long numer = (long long)first_num;
//     if (is_negative)
//       numer = -numer;
//     return (number){numer, 1ULL};
//   }
//
//   /* Если есть точка и точка — последний символ, считаем это целым (как в
//   Zig:
//    * "123." -> целое) */
//   if (str[end_first] == '.' && end_first + 1 == len) {
//     long long numer = (long long)first_num;
//     if (is_negative)
//       numer = -numer;
//     return (number){numer, 1ULL};
//   }
//
//   char next = str[end_first];
//
//   if (next == ' ') {
//     /* Смешанная дробь: <whole> <numerator>/<denominator>, допускаем
//     несколько
//      * пробелов */
//     size_t j = end_first;
//     while (j < len && str[j] == ' ')
//       ++j;
//     size_t start_num = j;
//     while (j < len && (unsigned char)str[j] >= '0' &&
//            (unsigned char)str[j] <= '9')
//       ++j;
//     size_t end_num = j;
//     if (j == len)
//       return (number){0, 0}; /* не может заканчиваться пробелом */
//     if (start_num == end_num)
//       return (number){0, 0}; /* пустой числитель */
//     if (str[j] != '/')
//       return (number){0, 0}; /* далее должна идти '/' */
//     /* парсим числитель */
//     unsigned long long numer_part;
//     if (!parse_u64_str(str + start_num, end_num - start_num, &numer_part))
//       return (number){0, 0};
//     /* парсим знаменатель */
//     ++j; /* пропустить '/' */
//     size_t start_den = j;
//     while (j < len && (unsigned char)str[j] >= '0' &&
//            (unsigned char)str[j] <= '9')
//       ++j;
//     size_t end_den = j;
//     if (j != len)
//       return (number){0, 0}; /* дальше ничего не должно быть */
//     if (start_den == end_den)
//       return (number){0, 0}; /* пустой знаменатель */
//     unsigned long long denom_part;
//     if (!parse_u64_str(str + start_den, end_den - start_den, &denom_part))
//       return (number){0, 0};
//     /* вычисляем итоговый числитель = whole * denom + numer_part */
//     /* проверяем возможное переполнение при умножении whole*denom */
//     if (first_num > 0 && denom_part > 0 && first_num > ULLONG_MAX /
//     denom_part)
//       return (number){0, 0};
//     unsigned long long abs_result_num_ull = first_num * denom_part +
//     numer_part; if (abs_result_num_ull > (unsigned long long)LLONG_MAX)
//       return (number){0, 0};
//     long long res_num = (long long)abs_result_num_ull;
//     if (is_negative)
//       res_num = -res_num;
//     return fractionRecovery((number){res_num, denom_part});
//   } else if (next == '.') {
//     /* Десятичная часть после целой */
//     size_t j = end_first + 1;
//     size_t start_dec = j;
//     while (j < len && (unsigned char)str[j] >= '0' &&
//            (unsigned char)str[j] <= '9')
//       ++j;
//     size_t end_dec = j;
//     if (start_dec == end_dec)
//       return (number){0, 0}; /* пустая дробная часть */
//     if (j != len)
//       return (number){0, 0}; /* дальше ничего быть не должно */
//     unsigned long long dec_part;
//     if (!parse_u64_str(str + start_dec, end_dec - start_dec, &dec_part))
//       return (number){0, 0};
//     unsigned long long denom;
//     size_t dec_len = end_dec - start_dec;
//     if (!pow10_u64(dec_len, &denom))
//       return (number){0, 0};
//     /* abs_numerator = dec_part + whole_part * denom  (проверяем
//     переполнение)
//      */
//     if (first_num > 0 && denom > 0 && first_num > ULLONG_MAX / denom)
//       return (number){0, 0};
//     unsigned long long abs_num_ull = dec_part + first_num * denom;
//     if (abs_num_ull > (unsigned long long)LLONG_MAX)
//       return (number){0, 0};
//     long long numer = (long long)abs_num_ull;
//     if (is_negative)
//       numer = -numer;
//     return fractionRecovery((number){numer, denom});
//   } else if (next == '/') {
//     /* Простая дробь вида A/B, где A — первый parsed (first_num) */
//     size_t j = end_first + 1;
//     size_t start_den = j;
//     while (j < len && (unsigned char)str[j] >= '0' &&
//            (unsigned char)str[j] <= '9')
//       ++j;
//     size_t end_den = j;
//     if (start_den == end_den)
//       return (number){0, 0}; /* пустой знаменатель */
//     if (j != len)
//       return (number){0, 0}; /* дальше ничего не должно быть */
//     unsigned long long denom;
//     if (!parse_u64_str(str + start_den, end_den - start_den, &denom))
//       return (number){0, 0};
//     if (first_num > (unsigned long long)LLONG_MAX)
//       return (number){0, 0};
//     long long numer = (long long)first_num;
//     if (is_negative)
//       numer = -numer;
//     return fractionRecovery((number){numer, denom});
//   } else {
//     return (number){0, 0}; /* формат не распознан */
//   }
// }

static bool parse_digits_ull(const char **pp, unsigned long long *out,
                             size_t *count) {
  const char *p = *pp;
  unsigned long long v = 0;
  size_t cnt = 0;
  unsigned char ch = (unsigned char)*p;
  if (ch < '0' || ch > '9')
    return false;
  while ((ch = (unsigned char)*p) >= '0' && ch <= '9') {
    unsigned digit = (unsigned)(ch - '0');
    if (v > (ULLONG_MAX - digit) / 10)
      return false; /* переполнение */
    v = v * 10 + digit;
    ++p;
    ++cnt;
  }
  *pp = p;
  *out = v;
  if (count)
    *count = cnt;
  return true;
}

/* Возвращает 10^n в out или false при переполнении */
static bool pow10_u64(size_t n, unsigned long long *out) {
  unsigned long long v = 1;
  for (size_t i = 0; i < n; ++i) {
    if (v > ULLONG_MAX / 10)
      return false;
    v *= 10;
  }
  *out = v;
  return true;
}

number aton(const char *str) {
  if (str == NULL)
    return (number){0, 0};

  const char *p = str;
  /* Нет пропуска начальных пробелов — ведём себя как Zig-версия (строго). */

  /* Сигн */
  bool is_negative = false;
  if (*p == '+' || *p == '-') {
    is_negative = (*p == '-');
    ++p;
    if (*p == '\0')
      return (number){0, 0}; /* только знак — ошибка */
  }

  /* Случай: .<digits> (десятичная часть без целой) */
  if (*p == '.') {
    ++p;
    const char *start_dec = p;
    unsigned long long dec_part;
    size_t dec_len;
    if (!parse_digits_ull(&p, &dec_part, &dec_len))
      return (number){0, 0}; /* пустая или переполнение */
    if (*p != '\0')
      return (number){0, 0}; /* дальше ничего не должно быть */
    unsigned long long denom;
    if (!pow10_u64(dec_len, &denom))
      return (number){0, 0};
    if (dec_part > (unsigned long long)LLONG_MAX)
      return (number){0, 0};
    long long numer = (long long)dec_part;
    if (is_negative)
      numer = -numer;
    return fractionRecovery((number){numer, denom});
  }

  /* Парсим целую часть (первая последовательность цифр) */
  unsigned long long whole;
  size_t whole_len;
  if (!parse_digits_ull(&p, &whole, &whole_len))
    return (number){0, 0};
  /* Если строка закончилась — целое число */
  if (*p == '\0') {
    if (whole > (unsigned long long)LLONG_MAX)
      return (number){0, 0};
    long long numer = (long long)whole;
    if (is_negative)
      numer = -numer;
    return (number){numer, 1ULL};
  }
  /* Если "123." (точка последний символ) — считаем целым */
  if (*p == '.' && p[1] == '\0') {
    if (whole > (unsigned long long)LLONG_MAX)
      return (number){0, 0};
    long long numer = (long long)whole;
    if (is_negative)
      numer = -numer;
    return (number){numer, 1ULL};
  }

  char ch = *p;
  if (ch == ' ') {
    /* Смешанная дробь: whole <spaces> numer/denom */
    /* пропустить пробелы */
    while (*p == ' ')
      ++p;
    /* парсим числитель */
    unsigned long long numer_part;
    size_t numer_len;
    if (!parse_digits_ull(&p, &numer_part, &numer_len))
      return (number){0, 0}; /* пустой или переполнение */
    if (*p != '/')
      return (number){0, 0}; /* далее должна идти '/' */
    ++p;                     /* пропустить '/' */
    unsigned long long denom;
    size_t denom_len;
    if (!parse_digits_ull(&p, &denom, &denom_len))
      return (number){0, 0}; /* пустой или переполнение */
    if (*p != '\0')
      return (number){0, 0}; /* дальше ничего не должно быть */

    /* Проверки на переполнение при whole * denom + numer_part */
    if (denom == 0)
      return (number){0, 0}; /* ноль в знаменателе — ошибка */
    if (whole > 0 && whole > ULLONG_MAX / denom)
      return (number){0, 0};
    unsigned long long abs_res = whole * denom;
    if (abs_res > ULLONG_MAX - numer_part)
      return (number){0, 0};
    abs_res += numer_part;
    if (abs_res > (unsigned long long)LLONG_MAX)
      return (number){0, 0};
    long long res_numer = (long long)abs_res;
    if (is_negative)
      res_numer = -res_numer;
    return fractionRecovery((number){res_numer, denom});
  } else if (ch == '.') {
    /* Десятичная часть после целой: whole.dec */
    ++p; /* переход к десятичной части */
    unsigned long long dec_part;
    size_t dec_len;
    if (!parse_digits_ull(&p, &dec_part, &dec_len))
      return (number){0, 0}; /* пустая или переполнение */
    if (*p != '\0')
      return (number){0, 0}; /* дальше ничего не должно быть */
    unsigned long long denom;
    if (!pow10_u64(dec_len, &denom))
      return (number){0, 0};
    if (denom == 0)
      return (number){0, 0};
    /* abs_numer = whole * denom + dec_part (проверки) */
    if (whole > 0 && whole > ULLONG_MAX / denom)
      return (number){0, 0};
    unsigned long long abs_num = whole * denom;
    if (abs_num > ULLONG_MAX - dec_part)
      return (number){0, 0};
    abs_num += dec_part;
    if (abs_num > (unsigned long long)LLONG_MAX)
      return (number){0, 0};
    long long numer = (long long)abs_num;
    if (is_negative)
      numer = -numer;
    return fractionRecovery((number){numer, denom});
  } else if (ch == '/') {
    /* Простая дробь: A/B (A == whole) */
    ++p; /* перейдём к знаменателю */
    unsigned long long denom;
    size_t denom_len;
    if (!parse_digits_ull(&p, &denom, &denom_len))
      return (number){0, 0};
    if (*p != '\0')
      return (number){0, 0};
    if (denom == 0)
      return (number){0, 0};
    if (whole > (unsigned long long)LLONG_MAX)
      return (number){0, 0};
    long long numer = (long long)whole;
    if (is_negative)
      numer = -numer;
    return fractionRecovery((number){numer, denom});
  } else {
    return (number){0, 0}; /* формат не распознан */
  }
}
