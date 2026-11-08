#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "fractions.h"

// 256KB на стеке для генерации гигантской строки без malloc
#define STACK_OUT_SIZE (262144)

static const char DIGIT_PAIRS[200] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0',
    '7', '0', '8', '0', '9', '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
    '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2',
    '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3',
    '7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
    '4', '5', '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5',
    '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6',
    '7', '6', '8', '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
    '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8', '0', '8', '1', '8',
    '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9',
    '7', '9', '8', '9', '9'};

static inline void write_bytes(char** __restrict ptr,
                               const char* __restrict s,
                               size_t len) {
  memcpy(*ptr, s, len);
  *ptr += len;
}

static inline void write_cstr(char** __restrict ptr, const char* __restrict s) {
  size_t len = strlen(s);
  memcpy(*ptr, s, len);
  *ptr += len;
}

static inline void write_ull(char** __restrict ptr, unsigned long long v) {
  if (v < 10) {
    *(*ptr)++ = (char)('0' + v);
    return;
  }
  if (v < 100) {
    memcpy(*ptr, &DIGIT_PAIRS[v * 2], 2);
    *ptr += 2;
    return;
  }
  char buf[32];
  char* p = buf + sizeof(buf);
  while (v >= 100) {
    unsigned int idx = (unsigned int)((v % 100) * 2);
    v /= 100;
    p -= 2;
    memcpy(p, &DIGIT_PAIRS[idx], 2);
  }
  if (v < 10) {
    *--p = (char)('0' + v);
  } else {
    p -= 2;
    memcpy(p, &DIGIT_PAIRS[v * 2], 2);
  }
  size_t len = (size_t)((buf + sizeof(buf)) - p);
  memcpy(*ptr, p, len);
  *ptr += len;
}

static inline void write_ull_padded(char** __restrict ptr,
                                    unsigned long long v,
                                    int pad) {
  char buf[32];
  char* p = buf + sizeof(buf);
  int len = 0;
  while (v >= 100) {
    unsigned int idx = (unsigned int)((v % 100) * 2);
    v /= 100;
    p -= 2;
    memcpy(p, &DIGIT_PAIRS[idx], 2);
    len += 2;
  }
  if (v < 10) {
    *--p = (char)('0' + v);
    len += 1;
  } else {
    p -= 2;
    memcpy(p, &DIGIT_PAIRS[v * 2], 2);
    len += 2;
  }
  while (len < pad) {
    *--p = '0';
    len++;
  }
  memcpy(*ptr, p, (size_t)len);
  *ptr += len;
}

static inline void write_ntoa(char** __restrict ptr, number num) {
  unsigned long long num_abs =
      (unsigned long long)__builtin_llabs(num.numerator);
  unsigned long long den = (unsigned long long)__builtin_llabs(num.denominator);
  bool is_neg = (num.numerator < 0) ^ (num.denominator < 0);

  if (is_neg && num_abs != 0)
    *(*ptr)++ = '-';

  if (__builtin_expect(den == 1, 1)) {
    write_ull(ptr, num_abs);
    return;
  }

  int twos = __builtin_ctzll(den);
  unsigned long long d_test = den >> twos;
  int fives = 0;
  while (d_test % 5 == 0) {
    d_test /= 5;
    fives++;
  }

  if (d_test == 1) {
    int decimal_places = (twos > fives) ? twos : fives;
    unsigned long long integer_part = num_abs / den;
    unsigned long long rem = num_abs % den;

    unsigned long long mul = 1;
    if (decimal_places > twos) {
      for (int i = 0; i < decimal_places - twos; i++)
        mul *= 5;
    } else if (decimal_places > fives) {
      mul <<= (decimal_places - fives);
    }
    unsigned long long decimal_part = rem * mul;

    write_ull(ptr, integer_part);
    *(*ptr)++ = '.';
    write_ull_padded(ptr, decimal_part, decimal_places);
  } else {
    unsigned long long integer_part = num_abs / den;
    unsigned long long rem = num_abs % den;
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

#if defined(ABSTRACT_ENABLED)
// Абстрактная сборка формулы (только склейка строк, без парсинга и математики)
static size_t formula_abstract(char* __restrict out,
                               char** __restrict args,
                               size_t size) {
  if (size == 0)
    return 0;
  char* ptr = out;

  if (size == 1) {
    write_cstr(&ptr, args[0]);
    *ptr++ = '\n';
    return (size_t)(ptr - out);
  }

  static const char PREFIX[] = "(0.5 \xc2\xb1 0.5)*(";
  for (size_t i = 0; i < size - 2; i++) {
    write_bytes(&ptr, PREFIX, sizeof(PREFIX) - 1);
  }

  if (size == 2) {
    write_bytes(&ptr, "(", 1);
    write_cstr(&ptr, args[0]);
    write_bytes(&ptr, "+", 1);
    write_cstr(&ptr, args[1]);
    write_bytes(&ptr, ")/2 \xc2\xb1 |", 8);
    write_cstr(&ptr, args[1]);
    write_bytes(&ptr, "-", 1);
    write_cstr(&ptr, args[0]);
    write_bytes(&ptr, "|/2", 3);
  } else {
    write_bytes(&ptr, "(", 1);
    write_cstr(&ptr, args[0]);
    write_bytes(&ptr, "+", 1);
    write_cstr(&ptr, args[1]);
    write_bytes(&ptr, "-2*", 3);
    write_cstr(&ptr, args[2]);
    write_bytes(&ptr, ")/2 \xc2\xb1 |", 8);
    write_cstr(&ptr, args[1]);
    write_bytes(&ptr, "-", 1);
    write_cstr(&ptr, args[0]);
    write_bytes(&ptr, "|/2", 3);

    for (size_t k = 2; k < size - 1; k++) {
      write_bytes(&ptr, ")+(", 3);
      write_cstr(&ptr, args[k]);
      write_bytes(&ptr, "-", 1);
      write_cstr(&ptr, args[k + 1]);
      write_bytes(&ptr, ")", 1);
    }
    write_bytes(&ptr, ")+", 2);
    write_cstr(&ptr, args[size - 1]);
  }

  *ptr++ = '\n';
  return (size_t)(ptr - out);
}
#else

// Экстремально быстрый кастомный QuickSort для инлайна
static inline int cmp_num(number a, number b) {
  __int128 left = (__int128)a.numerator * b.denominator;
  __int128 right = (__int128)b.numerator * a.denominator;
  if (left > right)
    return 1;
  if (left < right)
    return -1;
  return 0;
}

static inline void swap_num(number* a, number* b) {
  number tmp = *a;
  *a = *b;
  *b = tmp;
}

static void quicksort_nums(number* arr, int low, int high) {
  if (low < high) {
    number pivot = arr[high];
    int i = low - 1;
    for (int j = low; j <= high - 1; j++) {
      if (cmp_num(arr[j], pivot) < 0) {
        i++;
        swap_num(&arr[i], &arr[j]);
      }
    }
    swap_num(&arr[i + 1], &arr[high]);
    int pi = i + 1;

    quicksort_nums(arr, low, pi - 1);
    quicksort_nums(arr, pi + 1, high);
  }
}

// Оптимизированное удаление дубликатов (сортировка + фильтрация in-place)
static inline size_t remove_duplicates(number* nums, size_t size) {
  if (size <= 1)
    return size;
  quicksort_nums(nums, 0, (int)(size - 1));

  size_t new_size = 0;
  for (size_t i = 1; i < size; i++) {
    if (cmp_num(nums[i], nums[new_size]) != 0) {
      new_size++;
      nums[new_size] = nums[i];
    }
  }
  return new_size + 1;
}

// Математическая сборка с VLA (аллокация массивов прямо на стеке без malloc)
static size_t formula_math(char* __restrict out,
                           number* __restrict nums,
                           size_t size) {
  if (size == 0)
    return 0;
  char* ptr = out;

  if (size == 1) {
    write_ntoa(&ptr, nums[0]);
    *ptr++ = '\n';
    return (size_t)(ptr - out);
  }

  // Выделяем память на стеке за 0 наносекунд
  number saved[size];

  for (size_t k = size; k > 2; k--) {
    saved[k - 1] = nums[k - 1];
    for (size_t i = 0; i < k - 1; i++) {
      nums[i] = subn(nums[i], nums[k - 1]);
    }
  }

  static const char PREFIX[] = "(0.5 \xc2\xb1 0.5)*(";
  for (size_t i = 0; i < size - 2; i++) {
    write_bytes(&ptr, PREFIX, sizeof(PREFIX) - 1);
  }

  number divider = {2, 1};
  number n1 = divn(addn(nums[0], nums[1]), divider);
  number n2 = absn(divn(subn(nums[1], nums[0]), divider));

  write_ntoa(&ptr, n1);
  write_bytes(&ptr, " \xc2\xb1 ", 4);
  write_ntoa(&ptr, n2);

  for (size_t k = 3; k <= size; k++) {
    write_bytes(&ptr, ")", 1);
    if (isNegative(saved[k - 1])) {
      write_bytes(&ptr, "-", 1);
    } else {
      write_bytes(&ptr, "+", 1);
    }
    write_ntoa(&ptr, absn(saved[k - 1]));
  }

  *ptr++ = '\n';
  return (size_t)(ptr - out);
}
#endif  // ABSTRACT_ENABLED

int main(int argc, char** argv) {
  if (__builtin_expect(argc < 2, 0)) {
    printf("enter at least 1 number.\n example:\n ./main 1 2 3\n");
    return 0;
  }

  struct timespec start, mid, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  // Выделяем весь гигантский буфер сразу на стеке
  char out_buffer[STACK_OUT_SIZE];
  size_t out_len = 0;

#if defined(ABSTRACT_ENABLED)
  clock_gettime(CLOCK_MONOTONIC, &mid);
  out_len = formula_abstract(out_buffer, argv + 1, (size_t)(argc - 1));
  clock_gettime(CLOCK_MONOTONIC, &end);
#else
  size_t argc_sz = (size_t)(argc - 1);
  number nums[argc_sz];  // VLA - создание массива без malloc

  for (size_t i = 0; i < argc_sz; i++) {
    nums[i] = aton(argv[i + 1]);
  }
  size_t unique_size = remove_duplicates(nums, argc_sz);

  clock_gettime(CLOCK_MONOTONIC, &mid);
  out_len = formula_math(out_buffer, nums, unique_size);
  clock_gettime(CLOCK_MONOTONIC, &end);
#endif

#if defined(MEASURE_TIME_ENABLED)
  double duration =
      (end.tv_nsec - start.tv_nsec) + (end.tv_sec - start.tv_sec) * 1e9;
  double dur1 =
      (mid.tv_nsec - start.tv_nsec) + (mid.tv_sec - start.tv_sec) * 1e9;
  double dur2 = (end.tv_nsec - mid.tv_nsec) + (end.tv_sec - mid.tv_sec) * 1e9;
  duration /= 1000.0;
  dur1 /= 1000.0;
  dur2 /= 1000.0;

#if defined(ABSTRACT_ENABLED)
  // Добавлен суффикс us (микросекунды) для скрипта Python
  printf("elapsed:%.3fus\ncopy:%.3fus\nformula:%.3fus\n", duration, dur1, dur2);
#else
  // Добавлен суффикс us (микросекунды) для скрипта Python
  printf("elapsed:%.3fus\nparse:%.3fus\nformula:%.3fus\n", duration, dur1,
         dur2);
#endif
#endif

  // Быстрый прямой сброс в stdout минуя printf
  fwrite(out_buffer, 1, out_len, stdout);

  return 0;
}
