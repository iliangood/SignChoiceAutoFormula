#include <bits/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "fractions.h"

// Статический буфер для сверхбыстрой склейки строк (O(1) аллокаций)
#define OUT_BUFFER_SIZE (2048 * 1024)
static char out_buffer[OUT_BUFFER_SIZE];

static inline void write_str(char** ptr, const char* s) {
  while (*s)
    *(*ptr)++ = *s++;
}

// Сверхбыстрый конвертер чисел, обходит snprintf
static inline void write_ull(char** ptr, unsigned long long v) {
  char buf[32];
  char* p = buf + 31;
  int len = 0;
  do {
    *--p = '0' + (v % 10);
    v /= 10;
    len++;
  } while (v);
  memcpy(*ptr, p, len);
  *ptr += len;
}

static inline void write_ull_padded(char** ptr, unsigned long long v, int pad) {
  char buf[32];
  char* p = buf + 31;
  int len = 0;
  do {
    *--p = '0' + (v % 10);
    v /= 10;
    len++;
  } while (v);
  while (len < pad) {
    *--p = '0';
    len++;
  }
  memcpy(*ptr, p, len);
  *ptr += len;
}

static void write_ntoa(char** ptr, number num) {
  long long num_abs = llabs(num.numerator);
  long long den = llabs(num.denominator);
  bool is_neg = (num.numerator < 0) ^ (num.denominator < 0);

  if (den == 1) {
    if (is_neg && num_abs != 0)
      *(*ptr)++ = '-';
    write_ull(ptr, num_abs);
    return;
  }

  long long d_test = den;
  int twos = 0, fives = 0;
  while ((d_test & 1) == 0) {
    d_test >>= 1;
    twos++;
  }
  while (d_test % 5 == 0) {
    d_test /= 5;
    fives++;
  }

  if (d_test == 1) {
    if (is_neg && num_abs != 0)
      *(*ptr)++ = '-';
    int decimal_places = (twos > fives) ? twos : fives;
    long long integer_part = num_abs / den;
    long long rem = num_abs % den;

    long long mul = 1;
    for (int i = 0; i < decimal_places - twos; i++)
      mul *= 2;
    for (int i = 0; i < decimal_places - fives; i++)
      mul *= 5;
    long long decimal_part = rem * mul;

    write_ull(ptr, integer_part);
    *(*ptr)++ = '.';
    write_ull_padded(ptr, decimal_part, decimal_places);
  } else {
    if (is_neg && num_abs != 0)
      *(*ptr)++ = '-';
    long long integer_part = num_abs / den;
    long long rem = num_abs % den;
    if (integer_part > 0) {
      write_ull(ptr, integer_part);
      *(*ptr)++ = ' ';
      write_ull(ptr, rem);
      *(*ptr)++ = '/';
      write_ull(ptr, den);
    } else {
      write_ull(ptr, rem);
      *(*ptr)++ = '/';
      write_ull(ptr, den);
    }
  }
}

// Итеративная, безаллокационная сборка всей формулы
char* formula(number* nums, size_t size) {
  if (size < 1)
    return NULL;
  char* ptr = out_buffer;

  if (size == 1) {
    write_ntoa(&ptr, nums[0]);
    *ptr = '\0';
    return strdup(out_buffer);
  }

  number* saved = malloc(size * sizeof(number));

  // Фаза 1: предрасчет математики сверху-вниз
  for (size_t k = size; k > 2; k--) {
    saved[k - 1] = nums[k - 1];
    for (size_t i = 0; i < k - 1; i++) {
      nums[i] = subn(nums[i], nums[k - 1]);
    }
  }

  // Фаза 2: формирование строки слева-направо
  for (size_t i = 0; i < size - 2; i++) {
    write_str(&ptr, "(0.5 ± 0.5)*(");
  }

  number divider = {2, 1};
  number n1 = divn(addn(nums[0], nums[1]), divider);
  number n2 = absn(divn(subn(nums[1], nums[0]), divider));

  write_ntoa(&ptr, n1);
  write_str(&ptr, " ± ");
  write_ntoa(&ptr, n2);

  for (size_t k = 3; k <= size; k++) {
    write_str(&ptr, ")");
    if (isNegative(saved[k - 1]))
      write_str(&ptr, "-");
    else
      write_str(&ptr, "+");
    write_ntoa(&ptr, absn(saved[k - 1]));
  }

  *ptr = '\0';
  free(saved);

  char* res = malloc(ptr - out_buffer + 1);
  memcpy(res, out_buffer, ptr - out_buffer + 1);
  return res;
}

// 128-битное сравнение для исключения переполнений при кросс-умножении
static int compare(const void* pa, const void* pb) {
  number* a = (number*)pa;
  number* b = (number*)pb;
  __int128 left = (__int128)a->numerator * b->denominator;
  __int128 right = (__int128)b->numerator * a->denominator;
  if (left > right)
    return 1;
  if (left < right)
    return -1;
  return 0;
}

size_t remove_duplicates(number** nums, size_t size) {
  if (size == 0)
    return 0;
  qsort(*nums, size, sizeof(number), compare);
  size_t new_size = 0;
  for (size_t i = 1; i < size; i++) {
    if (compare(&(*nums)[i], &(*nums)[new_size]) != 0) {
      new_size++;
      (*nums)[new_size] = (*nums)[i];
    }
  }
  new_size++;
  return new_size;  // Мы можем не делать realloc ради скорости, памяти не жалко
}

int main(int argc, char** argv) {
  struct timespec start, mid, end;
  if (argc == 1) {
    printf("enter at least 1 number.\n example:\n ./main 1 2 3\n");
    return 0;
  }
  clock_gettime(CLOCK_MONOTONIC, &start);
  number* nums = malloc((argc - 1) * sizeof(number));
  for (int i = 1; i < argc; i++) {
    nums[i - 1] = aton(argv[i]);
  }
  size_t new_size = remove_duplicates(&nums, argc - 1);

  clock_gettime(CLOCK_MONOTONIC, &mid);
  char* res = formula(nums, new_size);
  clock_gettime(CLOCK_MONOTONIC, &end);

#if defined(MEASURE_TIME_ENABLED)
  double duration =
      (end.tv_nsec - start.tv_nsec) + (end.tv_sec - start.tv_sec) * 1e9;
  double dur1 =
      (mid.tv_nsec - start.tv_nsec) + (mid.tv_sec - start.tv_sec) * 1e9;
  double dur2 = (end.tv_nsec - mid.tv_nsec) + (end.tv_sec - mid.tv_sec) * 1e9;
  duration /= 1000.0;
  dur1 /= 1000.0;
  dur2 /= 1000.0;
  printf("duration:%.3fus\nparse:%.3fus\nformula:%.3fus\n", duration, dur1,
         dur2);
#endif

  printf("%s\n", res);
  free(nums);
  free(res);
  return 0;
}
