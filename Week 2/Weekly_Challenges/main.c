// 2026 S1C COMP2017 Week 1 Task 3.
// Tutor: Hao Ren (hao.ren@sydney.edu.au)


#include <stdio.h>
#include <limits.h>

int sum(int array[], unsigned int length) {
	int sum = 0;
	for (int i = 0; i < length; i++) {
		sum += array[i];
	}
	return sum;
}

int max(int array[], unsigned int length) {
	int max = array[0];
	for (int i = 0; i < length; i++) {
		if (max < array[i]) {
			max = array[i];
		}
	}
	return max;
}

int min(int array[], unsigned int length) {
	int min = array[0];
	for (int i = 0; i < length; i++) {
		if (min > array[i]) {
			min = array[i];
		}
	}
	return min;
}


int main() {
	unsigned int length = 0;
	scanf("%u", &length);
	int array[length];
	int temp = -1;
	for(int i = 0; i < length; i++) {
		scanf("%d", &temp);
		array[i] = temp;
	}

	printf("SUM: %d\n", sum(array, length));
	printf("MAX: %d\n", max(array, length));
	printf("MIN: %d\n", min(array, length));

	return 0;
}
