// compiled with : gcc -o memcpy memcpy.c -m32 -lm
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>

unsigned long long rdtsc(){
        asm("rdtsc");
}

char* slow_memcpy(char* dest, const char* src, size_t len){
	int i;
	for (i=0; i<len; i++) {
		dest[i] = src[i];
	}
	return dest;
}

char* fast_memcpy(char* dest, const char* src, size_t len){
	size_t i;
	// 64-byte block fast copy
	if(len >= 64){
		i = len / 64;
		len &= (64-1);
		while(i-- > 0){
			__asm__ __volatile__ (
			"movdqa (%0), %%xmm0\n"
			"movdqa 16(%0), %%xmm1\n"
			"movdqa 32(%0), %%xmm2\n"
			"movdqa 48(%0), %%xmm3\n"
			"movntps %%xmm0, (%1)\n"
			"movntps %%xmm1, 16(%1)\n"
			"movntps %%xmm2, 32(%1)\n"
			"movntps %%xmm3, 48(%1)\n"
			::"r"(src),"r"(dest):"memory");
			dest += 64;
			src += 64;
		}
	}

	// byte-to-byte slow copy
	if(len) slow_memcpy(dest, src, len);
	return dest;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Supply buffer size!\n");
		exit(0);
	}
	setvbuf(stdout, 0, _IONBF, 0);
	setvbuf(stdin, 0, _IOLBF, 0);

	unsigned long long t1, t2;
	int e;
	char* src;
	char* dest;
	unsigned int low, high;
	unsigned int size;
	// allocate memory
	char* cache1 = mmap(0, 0x4000, 7, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	char* cache2 = mmap(0, 0x4000, 7, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	src = mmap(0, 0x2000, 7, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	size_t sizes[10];
	int i=0;

	// run experiment
	size = atoi(argv[1]);
	printf("experiment: memcpy with buffer size %d\n",size);
	dest = malloc( size );

	memcpy(cache1, cache2, 0x4000);		// to eliminate cache effect
	t1 = rdtsc();
	slow_memcpy(dest, src, size);		// byte-to-byte memcpy
	t2 = rdtsc();
	printf("ellapsed CPU cycles for slow_memcpy : %llu\n", t2-t1);

	memcpy(cache1, cache2, 0x4000);		// to eliminate cache effect
	t1 = rdtsc();
	fast_memcpy(dest, src, size);		// block-to-block memcpy
	t2 = rdtsc();
	printf("ellapsed CPU cycles for fast_memcpy : %llu\n", t2-t1);
	printf("\n");

	return 0;
}

