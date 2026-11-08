#include "fractions.h"

static const unsigned long long POW10_LUT[20] = {1ULL,
                                                 10ULL,
                                                 100ULL,
                                                 1000ULL,
                                                 10000ULL,
                                                 100000ULL,
                                                 1000000ULL,
                                                 10000000ULL,
                                                 100000000ULL,
                                                 1000000000ULL,
                                                 10000000000ULL,
                                                 100000000000ULL,
                                                 1000000000000ULL,
                                                 10000000000000ULL,
                                                 100000000000000ULL,
                                                 1000000000000000ULL,
                                                 10000000000000000ULL,
                                                 100000000000000000ULL,
                                                 1000000000000000000ULL,
                                                 10000000000000000000ULL};

number aton(const char* __restrict str) {
  if (__builtin_expect(!str || !*str, 0))
    return (number){0, 1};

  bool is_neg = (*str == '-');
  if (*str == '+' || *str == '-')
    str++;
  if (__builtin_expect(!*str, 0))
    return (number){0, 1};

  if (*str == '.') {
    str++;
    unsigned long long val = 0;
    size_t digits = 0;
    while ((unsigned char)(*str - '0') <= 9) {
      val = val * 10 + (unsigned char)(*str - '0');
      digits++;
      str++;
    }
    unsigned long long mul = POW10_LUT[digits];
    return fractionRecovery(is_neg ? -(long long)val : (long long)val,
                            (long long)mul);
  }

  unsigned long long whole = 0;
  while ((unsigned char)(*str - '0') <= 9) {
    whole = whole * 10 + (unsigned char)(*str - '0');
    str++;
  }

  char c = *str;
  if (__builtin_expect(c == '\0', 1) || (c == '.' && str[1] == '\0')) {
    return (number){is_neg ? -(long long)whole : (long long)whole, 1};
  }

  if (c == '.') {
    str++;
    unsigned long long val = 0;
    size_t digits = 0;
    while ((unsigned char)(*str - '0') <= 9) {
      val = val * 10 + (unsigned char)(*str - '0');
      digits++;
      str++;
    }
    unsigned long long mul = POW10_LUT[digits];
    unsigned long long res_num = whole * mul + val;
    return fractionRecovery(is_neg ? -(long long)res_num : (long long)res_num,
                            (long long)mul);
  }

  if (c == '/') {
    str++;
    unsigned long long den = 0;
    while ((unsigned char)(*str - '0') <= 9) {
      den = den * 10 + (unsigned char)(*str - '0');
      str++;
    }
    return fractionRecovery(is_neg ? -(long long)whole : (long long)whole,
                            (long long)den);
  }

  if (c == ' ') {
    while (*str == ' ')
      str++;
    unsigned long long num = 0;
    while ((unsigned char)(*str - '0') <= 9) {
      num = num * 10 + (unsigned char)(*str - '0');
      str++;
    }
    if (*str == '/') {
      str++;
      unsigned long long den = 0;
      while ((unsigned char)(*str - '0') <= 9) {
        den = den * 10 + (unsigned char)(*str - '0');
        str++;
      }
      unsigned long long res_num = whole * den + num;
      return fractionRecovery(is_neg ? -(long long)res_num : (long long)res_num,
                              (long long)den);
    }
  }

  return (number){0, 1};
}
