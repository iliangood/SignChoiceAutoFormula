#include "fractions.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"
#include "math.h"
#include "string.h"
#include "ctype.h"

#define max(a,b) ((a>b)?(a):(b))
#define min(a,b) ((a<b)?(a):(b))

bool isCorrect(number num)
{
	return num.numerator;
}

long long llpow(long long num, long long power)
{
	if(power < 0)
		return 0;
	if(power == 0)
		return 1;
	bool sign = (power%2) && num < 0;
	long long res = 1;
	while(power >= 1)
	{
		if(power % 2)
		{
			power--;
			res *= num;
		}
		else
		{
			power /= 2;
			num *= num;
		}
	}
	return res;
}

long long gcd(long long a, long long b)
{
	a = llabs(a);
	b = llabs(b);
	while(max(a, b) % min(a, b))
	{
		if(a > b)
		{
			a = a % b;
		}
		else
		{
			b = b % a;
		}
	}
	return min(a, b);
}

number simplify(number num)
{
	int greatest_common_diviser = gcd(num.numerator, num.denominator);
	num.numerator /= greatest_common_diviser;
	num.denominator /= greatest_common_diviser;
	return num;
}

number signRecovery(number num)
{
	num.numerator *= (num.denominator < 0) ? -1 : 1;
	num.denominator = labs(num.denominator);
	return num;
}

number fractionRecovery(number num)
{
	return simplify(signRecovery(num));
}

number addn(number a, number b)
{
	number res;
	res.numerator = a.numerator * b.denominator + b.numerator * a.denominator;
	res.denominator = a.denominator * b.denominator;
	res = fractionRecovery(res);
	return res;
}

number subn(number a, number b)
{
	number res;
	res.numerator = a.numerator * b.denominator - b.numerator * a.denominator;
	res.denominator = a.denominator * b.denominator;
	res = fractionRecovery(res);
	return res;
}

number muln(number a, number b)
{
	number res;
	res.numerator = a.numerator * b.numerator;
	res.denominator = a.denominator * b.denominator;
	res = fractionRecovery(res);
	return res;
}

number divn(number a, number b)
{
	number res;
	res.numerator = a.numerator * b.denominator;
	res.denominator = a.denominator * b.numerator;
	res = fractionRecovery(res);
	return res;
}

number absn(number num)
{
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

number iton(long long num)
{
	number res;
	res.numerator = num;
	res.denominator = 1;
	return res;
}

bool isNegative(number num)
{
	num = fractionRecovery(num);
	return num.numerator < 0;
}

char* ntoda(number num) {
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

	char* result = (char*)malloc(buffer_size * sizeof(char));
	if (result == NULL) {
		return NULL;
	}

	if (decimal_places == 0) {
		snprintf(result, buffer_size, "%s%lld", is_negative ? "-" : "", num.numerator / num.denominator);
	} else {
		char format[20];
		snprintf(format, 20, "%%s%%lld.%%0%dlld", decimal_places);
		long long ten_pow = llpow(10, decimal_places);
		//for (int i = 0; i < decimal_places; i++) ten_pow *= 10;
		long long decimal_part = (num.numerator % num.denominator) * ((twos > fives) ? llpow(5, twos - fives) : llpow(2, fives - twos));
		long long integer_part = num.numerator / num.denominator;
		snprintf(result, buffer_size, format, is_negative ? "-" : "", integer_part, decimal_part);
	}

	size_t actual_size = strlen(result) + 1;
	char* resized_result = (char*)realloc(result, actual_size * sizeof(char));
	if (resized_result == NULL) {
		return result;
	}
	return resized_result;
}

char* ntosfa(number num)
{
	num = fractionRecovery(num);
	char* str = malloc(60);
	if(str == NULL)
		return NULL;
	snprintf(str, 60, "%lld/%lld", num.numerator, num.denominator);
	char* res = realloc(str, strlen(str)+1);
	if(res == NULL)
		return str;
	return res;
}

char* ntofwi(number num)
{
	num = fractionRecovery(num);
	char* str = malloc(60);
	if(str == NULL)
		return NULL;
	long long integer_part = num.numerator / num.denominator;
	snprintf(str, 60, "%lld %lld/%lld", integer_part, num.numerator % num.denominator, num.denominator);
	char* res = realloc(str, strlen(str)+1);
	if(res == NULL)
		return str;
	return res;
}

char* ntofa(number num)
{
	num = fractionRecovery(num);
	long long integer_part = num.numerator / num.denominator;
	char* str = malloc(60);
	if(integer_part)
		return ntofwi(num);
	return ntosfa(num);
}

char* ntoa(number num)
{
	num = fractionRecovery(num);
	char* res = ntoda(num);
	if(res != NULL)
		return res;
	return ntofa(num);
}

number aton(char* str)
{
	if(str == NULL)
		return (number){0, 0};
	long long integer_part, numerator, denominator;
	char next_char = 0;
	int test = sscanf(str, "%lld%c", &integer_part, &next_char);
	if(test == 1 || test == 2)
		if(next_char == 0 || isspace(next_char))
			return (number){integer_part, 1};
	if(sscanf(str, "%lld+%lld/%lld", &integer_part, &numerator, &denominator) == 3)
		return fractionRecovery((number){integer_part*denominator + numerator, denominator});
	if(sscanf(str, "%lld/%lld", &numerator, &denominator) == 2)
		return fractionRecovery((number){numerator, denominator});
	char* dec = malloc(30);
	if(dec == NULL)
		return (number){0, 0};
	if(sscanf(str, "%lld.%s", &integer_part, dec) == 2)
	{
		denominator = llpow(10, strlen(dec));
		numerator = atoi(dec) + integer_part*denominator;
		return fractionRecovery((number){numerator, denominator});
	}
	free(dec);
	return (number){0, 0};
}

