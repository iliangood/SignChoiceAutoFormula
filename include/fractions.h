#if !defined(FRACTIONS_H)
#define FRACTIONS_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct number {
  long long numerator;
  long long denominator;
} number;

static inline long long gcd(long long a, long long b) {
  if (a == 0)
    return __builtin_llabs(b);
  if (b == 0)
    return __builtin_llabs(a);

  unsigned long long ua = (unsigned long long)__builtin_llabs(a);
  unsigned long long ub = (unsigned long long)__builtin_llabs(b);

  int shift = __builtin_ctzll(ua | ub);
  ua >>= __builtin_ctzll(ua);

  do {
    ub >>= __builtin_ctzll(ub);
    if (ua > ub) {
      unsigned long long t = ub;
      ub = ua;
      ua = t;
    }
    ub -= ua;
  } while (ub != 0);

  return (long long)(ua << shift);
}

static inline number fractionRecovery(long long num, long long den) {
  if (den < 0) {
    num = -num;
    den = -den;
  }
  if (den == 1)
    return (number){num, 1};
  if (num == 0)
    return (number){0, 1};
  long long g = gcd(num, den);
  return (number){num / g, den / g};
}

static inline number addn(number a, number b) {
  if (__builtin_expect(a.denominator == b.denominator, 1)) {
    if (a.denominator == 1)
      return (number){a.numerator + b.numerator, 1};
    return fractionRecovery(a.numerator + b.numerator, a.denominator);
  }
  return fractionRecovery(
      a.numerator * b.denominator + b.numerator * a.denominator,
      a.denominator * b.denominator);
}

static inline number subn(number a, number b) {
  if (__builtin_expect(a.denominator == b.denominator, 1)) {
    if (a.denominator == 1)
      return (number){a.numerator - b.numerator, 1};
    return fractionRecovery(a.numerator - b.numerator, a.denominator);
  }
  return fractionRecovery(
      a.numerator * b.denominator - b.numerator * a.denominator,
      a.denominator * b.denominator);
}

static inline number muln(number a, number b) {
  if (a.denominator == 1 && b.denominator == 1)
    return (number){a.numerator * b.numerator, 1};
  return fractionRecovery(a.numerator * b.numerator,
                          a.denominator * b.denominator);
}

static inline number divn(number a, number b) {
  return fractionRecovery(a.numerator * b.denominator,
                          a.denominator * b.numerator);
}

static inline number div2n(number a) {
  if ((a.numerator & 1) == 0) {
    return (number){a.numerator >> 1, a.denominator};
  }
  return (number){a.numerator, a.denominator << 1};
}

static inline number absn(number num) {
  return (number){__builtin_llabs(num.numerator),
                  __builtin_llabs(num.denominator)};
}

static inline number iton(long long num) {
  return (number){num, 1};
}

static inline bool isNegative(number num) {
  return (num.numerator < 0) ^ (num.denominator < 0);
}

number aton(const char* __restrict str);

#endif
