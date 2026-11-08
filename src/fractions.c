#include "fractions.h"
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max(a, b) ((a > b) ? (a) : (b))
#define min(a, b) ((a < b) ? (a) : (b))

// Сверхбыстрый бинарный алгоритм (алгоритм Стейна), использующий аппаратные
// инструкции
long long gcd(long long a, long long b) {
  if (a == 0)
    return __builtin_llabs(b);
  if (b == 0)
    return __builtin_llabs(a);

  a = __builtin_llabs(a);
  b = __builtin_llabs(b);

  int shift = __builtin_ctzll(a | b);
  a >>= __builtin_ctzll(a);

  do {
    b >>= __builtin_ctzll(b);
    if (a > b) {
      long long t = b;
      b = a;
      a = t;
    }
    b -= a;
  } while (b != 0);

  return a << shift;
}

static inline number fractionRecovery(long long num, long long den) {
  if (den < 0) {
    num = -num;
    den = -den;
  }
  long long g = gcd(num, den);
  return (number){num / g, den / g};
}

number addn(number a, number b) {
  return fractionRecovery(
      a.numerator * b.denominator + b.numerator * a.denominator,
      a.denominator * b.denominator);
}

number subn(number a, number b) {
  return fractionRecovery(
      a.numerator * b.denominator - b.numerator * a.denominator,
      a.denominator * b.denominator);
}

number muln(number a, number b) {
  return fractionRecovery(a.numerator * b.numerator,
                          a.denominator * b.denominator);
}

number divn(number a, number b) {
  return fractionRecovery(a.numerator * b.denominator,
                          a.denominator * b.numerator);
}

number absn(number num) {
  num.numerator = llabs(num.numerator);
  num.denominator = llabs(num.denominator);
  return num;
}

number iton(long long num) {
  return (number){num, 1};
}

bool isNegative(number num) {
  // Поскольку у нас числитель и знаменатель упрощаются, достаточно проверить
  // знаки
  return (num.numerator < 0) ^ (num.denominator < 0);
}

// Заглушка, ntoa теперь вшит напрямую в main для максимальной скорости вывода
// без промежуточных malloc
char* ntoa(number num) {
  return NULL;
}

// Оптимизированный быстрый парсер строки напрямую из памяти без копирований и
// strlen
number aton(const char* str) {
  if (!str || !*str)
    return (number){0, 1};

  bool is_neg = (*str == '-');
  if (*str == '+' || *str == '-')
    str++;
  if (!*str)
    return (number){0, 1};

  if (*str == '.') {
    str++;
    unsigned long long val = 0;
    unsigned long long mul = 1;
    while (*str >= '0' && *str <= '9') {
      val = val * 10 + (*str - '0');
      mul *= 10;
      str++;
    }
    return fractionRecovery(is_neg ? -val : val, mul);
  }

  unsigned long long whole = 0;
  while (*str >= '0' && *str <= '9') {
    whole = whole * 10 + (*str - '0');
    str++;
  }

  if (*str == '\0' || (*str == '.' && *(str + 1) == '\0')) {
    return (number){is_neg ? -whole : whole, 1};
  }

  if (*str == ' ') {
    while (*str == ' ')
      str++;
    unsigned long long num = 0;
    while (*str >= '0' && *str <= '9') {
      num = num * 10 + (*str - '0');
      str++;
    }
    if (*str == '/') {
      str++;
      unsigned long long den = 0;
      while (*str >= '0' && *str <= '9') {
        den = den * 10 + (*str - '0');
        str++;
      }
      unsigned long long res_num = whole * den + num;
      return fractionRecovery(is_neg ? -res_num : res_num, den);
    }
    return (number){0, 1};
  }

  if (*str == '.') {
    str++;
    unsigned long long val = 0;
    unsigned long long mul = 1;
    while (*str >= '0' && *str <= '9') {
      val = val * 10 + (*str - '0');
      mul *= 10;
      str++;
    }
    unsigned long long res_num = whole * mul + val;
    return fractionRecovery(is_neg ? -res_num : res_num, mul);
  }

  if (*str == '/') {
    str++;
    unsigned long long den = 0;
    while (*str >= '0' && *str <= '9') {
      den = den * 10 + (*str - '0');
      str++;
    }
    return fractionRecovery(is_neg ? -whole : whole, den);
  }

  return (number){0, 1};
}
