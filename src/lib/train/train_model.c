#include <train_model.h>

int sqrt(int x)
{
	unsigned int a = 0L;
	unsigned int r = 0L;
	unsigned int e = 0L;

	int i;

	for (i = 0; i < BITSPERINT; i++)
	{
	    r = (r << 2) + TOP2BITS(x); x <<= 2;
	    a <<= 1;
	    e = (a << 1) + 1;
	    if (r >= e)
	    {
	          r -= e;
	          a++;
	    }
	}

	return a/65535;
}

int mean(int * list, int size){
	int sum = 0;
	for(int i = 0; i < size; i++){
		sum += list[i];
	}

	return (sum/size);
}

int standard_deviation(int *list, int size){
	int diff;
	int inter_sum = 0;
	int mean = mean(list, size);

	for(int i = 0; i < sie; i++){
		diff = list[i] - mean;
		inter_sum += diff * diff;
	}

	inter_sum = inter_sum / (size - 1);

	return sqrt(inter_sum);
}