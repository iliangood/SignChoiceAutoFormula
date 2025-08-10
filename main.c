#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "fractions.h"

int getCharsN(int number) {
	bool invertedSign = number < 0;
	if (number == 0) return 1; // Учитываем случай, когда число равно 0
	number = abs(number); // Учитываем отрицательные числа
	int count = 0;
	while (number > 0) {
		number /= 10;
		count++;
	}
	return count + invertedSign;
}

char* formula(number* nums, size_t size)
{
	if(size < 1)
		return NULL;
	if(size == 1)
	{
		char* buf = ntoa(nums[0]);
		return buf;
	}
	if(size == 2)
	{
		number divider = {2,1};
		char* num1 = ntoa(divn(addn(nums[0], nums[1]), divider));
		if(num1 == NULL)
			return NULL;
		char* num2 = ntoa(absn(divn(subn(nums[1], nums[0]), divider)));
		if(num2 == NULL)
		{
			free(num1);
			return NULL;
		}
		size_t length = strlen(num1) + strlen(num2) + 5;
		char* buf = malloc(length*sizeof(char));
		if(buf == NULL)
		{
			free(num1);
			free(num2);
			return NULL;
		}
		snprintf(buf, length, "%s ± %s", num1, num2);
		free(num1);
		free(num2);
		return buf;
	}
	for(int i = 0; i < size - 1; i++)
		nums[i] = subn(nums[i], nums[size-1]);
	char* recurse = formula(nums, size-1);
	if(recurse == NULL)
		return NULL;
	char* num = ntoa(absn(nums[size-1]));
	if(num == NULL)
	{
		free(recurse);
		return NULL;
	}
	size_t length = strlen(recurse) + strlen(num) + 1 + 16;
	char* buf = malloc(length*sizeof(char));
	snprintf(buf, length, "(0.5 ± 0.5)*(%s)%c%s", recurse, isNegative(nums[size-1]) ? '-':'+', num);
	free(recurse);
	free(num);
	return buf;
}

/*int main()
{
	number num;
	for(num.numerator = 1; num.numerator < 20; num.numerator++)
		for(num.denominator = 1; num.denominator < 20; num.denominator++)
		{
			char* str = ntoa(num);
			if(str == NULL)
				break;
			printf("  %lld/%lld = %s\n", num.numerator, num.denominator, str);
			free(str);
		}
	return 0;
}*/

/*int main()
{
	size_t size = 4;
	number* arr = malloc(size * sizeof(number));
	for(int i = 0; i < size; i++)
		arr[i] = iton(i+1);
	char* res = formula(arr, size);
	printf("  %s\n", res);
	return 0;
}*/

int compare(const void* a, const void* b)
{
	return cmpn(*(number*)a, *(number*)b);
}

size_t remove_duplicates(number** nums, size_t size)
{
	qsort(*nums, size, sizeof(number), compare);
	size_t new_size = 0;
	for(int i = 0; i < size; i++)
	{
		if(cmpn((*nums)[i], (*nums)[new_size]) != 0)
		{
			new_size++;
			(*nums)[new_size] = (*nums)[i];
		}
	}
	new_size++;
	number* new_arr = realloc(*nums, new_size*sizeof(number));
	if(new_arr == NULL)
		return new_size;
	*nums = new_arr;
	return new_size;
}

int main(int argc, char** argv)
{
	if(argc == 1)
	{
		printf("enter at least 1 number.\n example:\n ./main 1 2 3");
	}
	number* nums = malloc((argc-1)*sizeof(number));
	for(int i = 1; i < argc; i++)
	{
		nums[i-1] = aton(argv[i]);
	}
	size_t new_size = remove_duplicates(&nums, argc-1);
	printf(" %s\n", formula(nums, new_size));
	free(nums);
	return 0;
}
