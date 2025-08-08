#if !defined(FRACTIONS_H)
#define FRACTIONS_H
#include "stdbool.h"
long long gcd(long long a, long long b);
typedef struct number
{
	long long numerator;
	long long denominator;
} number;
number addn(number a, number b);
number subn(number a, number b);
number muln(number a, number b);
number divn(number a, number b);
number absn(number num);
number iton(long long num);
bool isNegative(number num);
char* ntoda(number num);
char* ntosfa(number num);
char* ntofwi(number num);
char* ntifa(number num);
char* ntoa(number num);
number aton(char* num);
#endif
