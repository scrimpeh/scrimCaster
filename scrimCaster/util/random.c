#include <util/random.h>

#include <stdlib.h>

void random_srand(i32 seed)
{
	srand(seed);
}

i32 random_rand()
{
	return rand();
}