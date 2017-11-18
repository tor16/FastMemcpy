//=====================================================================
//
// FastMemcpy.c - skywind3000@163.com, 2015
//
// feature:
// 50% speed up in avg. vs standard memcpy (tested in vc2012/gcc4.9)
//
//=====================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "FastMemcpy.h"

#ifndef _WIN32
static  uint64_t timeGetTime(void)    { struct timespec tm; clock_gettime(CLOCK_MONOTONIC, &tm); return (uint64_t)tm.tv_sec*1000000ull + tm.tv_nsec/1000; }
#endif

void benchmark(int dstalign, int srcalign, size_t size, int times)
{
	char *DATA1 = (char*)malloc(size + 64);
	char *DATA2 = (char*)malloc(size + 64);
	size_t LINEAR1 = ((size_t)DATA1);
	size_t LINEAR2 = ((size_t)DATA2);
	char *ALIGN1 = (char*)(((64 - (LINEAR1 & 63)) & 63) + LINEAR1);
	char *ALIGN2 = (char*)(((64 - (LINEAR2 & 63)) & 63) + LINEAR2);
	char *dst = (dstalign)? ALIGN1 : (ALIGN1 + 1);
	char *src = (srcalign)? ALIGN2 : (ALIGN2 + 3);
#ifdef _WIN32
	DWORD t1, t2;
#else
	uint64_t t1, t2;
#endif
	int k;
	
	sleep(100);
	t1 = timeGetTime();
	for (k = times; k > 0; k--) {
		memcpy(dst, src, size);
	}
	t1 = timeGetTime() - t1;
	sleep(100);
	t2 = timeGetTime();
	for (k = times; k > 0; k--) {
		memcpy_fast(dst, src, size);
	}
	t2 = timeGetTime() - t2;

	free(DATA1);
	free(DATA2);

	printf("result(dst %s, src %s): memcpy_fast=%dms memcpy=%d ms\n",  
		dstalign? "aligned" : "unalign", 
		srcalign? "aligned" : "unalign", (int)t2, (int)t1);
}


void bench(int copysize, int times)
{
	printf("benchmark(size=%d bytes, times=%d):\n", copysize, times);
	benchmark(1, 1, copysize, times);
	benchmark(1, 0, copysize, times);
	benchmark(0, 1, copysize, times);
	benchmark(0, 0, copysize, times);
	printf("\n");
}


void random_bench(int maxsize, int times)
{
	static char A[11 * 1024 * 1024 + 2];
	static char B[11 * 1024 * 1024 + 2];
	static int random_offsets[0x10000];
	static int random_sizes[0x8000];
	unsigned int i, p1, p2;
#ifdef _win32
	DWORD t1, t2;
#else
	uint64_t t1, t2;
#endif
	for (i = 0; i < 0x10000; i++) {	// generate random offsets
		random_offsets[i] = rand() % (10 * 1024 * 1024 + 1);
	}
	for (i = 0; i < 0x8000; i++) {	// generate random sizes
		random_sizes[i] = 1 + rand() % maxsize;
	}
	sleep(100);
	t1 = timeGetTime();
	for (p1 = 0, p2 = 0, i = 0; i < times; i++) {
		int offset1 = random_offsets[(p1++) & 0xffff];
		int offset2 = random_offsets[(p1++) & 0xffff];
		int size = random_sizes[(p2++) & 0x7fff];
		memcpy(A + offset1, B + offset2, size);
	}
	t1 = timeGetTime() - t1;
	sleep(100);
	t2 = timeGetTime();
	for (p1 = 0, p2 = 0, i = 0; i < times; i++) {
		int offset1 = random_offsets[(p1++) & 0xffff];
		int offset2 = random_offsets[(p1++) & 0xffff];
		int size = random_sizes[(p2++) & 0x7fff];
		memcpy_fast(A + offset1, B + offset2, size);
	}
	t2 = timeGetTime() - t2;
	printf("benchmark random access:\n");
	printf("memcpy_fast=%dms memcpy=%dms\n\n", (int)t2, (int)t1);
}


#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

int main(void)
{
	bench(32, 0x1000000);
	bench(64, 0x1000000);
	bench(512, 0x800000);
	bench(1024, 0x400000);
	bench(4096, 0x80000);
	bench(8192, 0x40000);
	bench(1024 * 1024 * 1, 0x800);
	bench(1024 * 1024 * 4, 0x200);
	bench(1024 * 1024 * 8, 0x100);
	
	random_bench(2048, 8000000);

	return 0;
}




/*
benchmark(size=32 bytes, times=16777216):
result(dst aligned, src aligned): memcpy_fast=78ms memcpy=260 ms
result(dst aligned, src unalign): memcpy_fast=78ms memcpy=250 ms
result(dst unalign, src aligned): memcpy_fast=78ms memcpy=266 ms
result(dst unalign, src unalign): memcpy_fast=78ms memcpy=234 ms

benchmark(size=64 bytes, times=16777216):
result(dst aligned, src aligned): memcpy_fast=109ms memcpy=281 ms
result(dst aligned, src unalign): memcpy_fast=109ms memcpy=328 ms
result(dst unalign, src aligned): memcpy_fast=109ms memcpy=343 ms
result(dst unalign, src unalign): memcpy_fast=93ms memcpy=344 ms

benchmark(size=512 bytes, times=8388608):
result(dst aligned, src aligned): memcpy_fast=125ms memcpy=218 ms
result(dst aligned, src unalign): memcpy_fast=156ms memcpy=484 ms
result(dst unalign, src aligned): memcpy_fast=172ms memcpy=546 ms
result(dst unalign, src unalign): memcpy_fast=172ms memcpy=515 ms

benchmark(size=1024 bytes, times=4194304):
result(dst aligned, src aligned): memcpy_fast=109ms memcpy=172 ms
result(dst aligned, src unalign): memcpy_fast=187ms memcpy=453 ms
result(dst unalign, src aligned): memcpy_fast=172ms memcpy=437 ms
result(dst unalign, src unalign): memcpy_fast=156ms memcpy=452 ms

benchmark(size=4096 bytes, times=524288):
result(dst aligned, src aligned): memcpy_fast=62ms memcpy=78 ms
result(dst aligned, src unalign): memcpy_fast=109ms memcpy=202 ms
result(dst unalign, src aligned): memcpy_fast=94ms memcpy=203 ms
result(dst unalign, src unalign): memcpy_fast=110ms memcpy=218 ms

benchmark(size=8192 bytes, times=262144):
result(dst aligned, src aligned): memcpy_fast=62ms memcpy=78 ms
result(dst aligned, src unalign): memcpy_fast=78ms memcpy=202 ms
result(dst unalign, src aligned): memcpy_fast=78ms memcpy=203 ms
result(dst unalign, src unalign): memcpy_fast=94ms memcpy=203 ms

benchmark(size=1048576 bytes, times=2048):
result(dst aligned, src aligned): memcpy_fast=203ms memcpy=191 ms
result(dst aligned, src unalign): memcpy_fast=219ms memcpy=281 ms
result(dst unalign, src aligned): memcpy_fast=218ms memcpy=328 ms
result(dst unalign, src unalign): memcpy_fast=218ms memcpy=312 ms

benchmark(size=4194304 bytes, times=512):
result(dst aligned, src aligned): memcpy_fast=312ms memcpy=406 ms
result(dst aligned, src unalign): memcpy_fast=296ms memcpy=421 ms
result(dst unalign, src aligned): memcpy_fast=312ms memcpy=468 ms
result(dst unalign, src unalign): memcpy_fast=297ms memcpy=452 ms

benchmark(size=8388608 bytes, times=256):
result(dst aligned, src aligned): memcpy_fast=281ms memcpy=452 ms
result(dst aligned, src unalign): memcpy_fast=280ms memcpy=468 ms
result(dst unalign, src aligned): memcpy_fast=298ms memcpy=514 ms
result(dst unalign, src unalign): memcpy_fast=344ms memcpy=472 ms

benchmark random access:
memcpy_fast=515ms memcpy=1014ms

*/

/*
Tested in Lenovo Y560 laptop, Ubuntu16.04, Linux 4.10.0-38-generic, gcc 5.4.0

benchmark(size=32 bytes, times=16777216):
result(dst aligned, src aligned): memcpy_fast=90610ms memcpy=90236 ms
result(dst aligned, src unalign): memcpy_fast=89426ms memcpy=90462 ms
result(dst unalign, src aligned): memcpy_fast=87669ms memcpy=88948 ms
result(dst unalign, src unalign): memcpy_fast=89902ms memcpy=89833 ms

benchmark(size=64 bytes, times=16777216):
result(dst aligned, src aligned): memcpy_fast=88327ms memcpy=125963 ms
result(dst aligned, src unalign): memcpy_fast=89225ms memcpy=209482 ms
result(dst unalign, src aligned): memcpy_fast=88470ms memcpy=211084 ms
result(dst unalign, src unalign): memcpy_fast=90099ms memcpy=211230 ms

benchmark(size=512 bytes, times=8388608):
result(dst aligned, src aligned): memcpy_fast=171732ms memcpy=421256 ms
result(dst aligned, src unalign): memcpy_fast=179523ms memcpy=423692 ms
result(dst unalign, src aligned): memcpy_fast=174490ms memcpy=423971 ms
result(dst unalign, src unalign): memcpy_fast=184931ms memcpy=421675 ms

benchmark(size=1024 bytes, times=4194304):
result(dst aligned, src aligned): memcpy_fast=148628ms memcpy=397249 ms
result(dst aligned, src unalign): memcpy_fast=176322ms memcpy=397741 ms
result(dst unalign, src aligned): memcpy_fast=195281ms memcpy=401186 ms
result(dst unalign, src unalign): memcpy_fast=178921ms memcpy=399711 ms

benchmark(size=4096 bytes, times=524288):
result(dst aligned, src aligned): memcpy_fast=75860ms memcpy=193065 ms
result(dst aligned, src unalign): memcpy_fast=88990ms memcpy=197428 ms
result(dst unalign, src aligned): memcpy_fast=91888ms memcpy=197137 ms
result(dst unalign, src unalign): memcpy_fast=90846ms memcpy=195016 ms

benchmark(size=8192 bytes, times=262144):
result(dst aligned, src aligned): memcpy_fast=74233ms memcpy=191768 ms
result(dst aligned, src unalign): memcpy_fast=92444ms memcpy=193460 ms
result(dst unalign, src aligned): memcpy_fast=93219ms memcpy=191124 ms
result(dst unalign, src unalign): memcpy_fast=90983ms memcpy=191684 ms

benchmark(size=1048576 bytes, times=2048):
result(dst aligned, src aligned): memcpy_fast=164173ms memcpy=164695 ms
result(dst aligned, src unalign): memcpy_fast=159651ms memcpy=188449 ms
result(dst unalign, src aligned): memcpy_fast=163897ms memcpy=187896 ms
result(dst unalign, src unalign): memcpy_fast=164621ms memcpy=187325 ms

benchmark(size=4194304 bytes, times=512):
result(dst aligned, src aligned): memcpy_fast=616944ms memcpy=599963 ms
result(dst aligned, src unalign): memcpy_fast=488641ms memcpy=582817 ms
result(dst unalign, src aligned): memcpy_fast=529188ms memcpy=628512 ms
result(dst unalign, src unalign): memcpy_fast=542788ms memcpy=634394 ms

benchmark(size=8388608 bytes, times=256):
result(dst aligned, src aligned): memcpy_fast=662827ms memcpy=627687 ms
result(dst aligned, src unalign): memcpy_fast=528750ms memcpy=633706 ms
result(dst unalign, src aligned): memcpy_fast=585210ms memcpy=635820 ms
result(dst unalign, src unalign): memcpy_fast=599523ms memcpy=665385 ms

benchmark random access:
memcpy_fast=2743385ms memcpy=2849728ms
*/

/*
Tested in Dell Optiplex 7050SFF desktop, Ubuntu16.04, Linux 4.10.0-38-generic,
gcc 5.4.0, CPU: Intel(R) Core(TM) i7-7700 CPU @ 3.60GHz

benchmark(size=32 bytes, times=16777216):
result(dst aligned, src aligned): memcpy_fast=36517ms memcpy=40759 ms
result(dst aligned, src unalign): memcpy_fast=35850ms memcpy=40335 ms
result(dst unalign, src aligned): memcpy_fast=37174ms memcpy=40285 ms
result(dst unalign, src unalign): memcpy_fast=36474ms memcpy=41329 ms

benchmark(size=64 bytes, times=16777216):
result(dst aligned, src aligned): memcpy_fast=36290ms memcpy=44864 ms
result(dst aligned, src unalign): memcpy_fast=36230ms memcpy=44156 ms
result(dst unalign, src aligned): memcpy_fast=65311ms memcpy=48575 ms
result(dst unalign, src unalign): memcpy_fast=36395ms memcpy=47987 ms

benchmark(size=512 bytes, times=8388608):
result(dst aligned, src aligned): memcpy_fast=74848ms memcpy=48318 ms
result(dst aligned, src unalign): memcpy_fast=79708ms memcpy=48678 ms
result(dst unalign, src aligned): memcpy_fast=78697ms memcpy=52363 ms
result(dst unalign, src unalign): memcpy_fast=79717ms memcpy=52188 ms

benchmark(size=1024 bytes, times=4194304):
result(dst aligned, src aligned): memcpy_fast=72423ms memcpy=42238 ms
result(dst aligned, src unalign): memcpy_fast=77820ms memcpy=42348 ms
result(dst unalign, src aligned): memcpy_fast=78111ms memcpy=44610 ms
result(dst unalign, src unalign): memcpy_fast=105127ms memcpy=44698 ms

benchmark(size=4096 bytes, times=524288):
result(dst aligned, src aligned): memcpy_fast=37105ms memcpy=26054 ms
result(dst aligned, src unalign): memcpy_fast=40146ms memcpy=26132 ms
result(dst unalign, src aligned): memcpy_fast=70361ms memcpy=34507 ms
result(dst unalign, src unalign): memcpy_fast=40138ms memcpy=68790 ms

benchmark(size=8192 bytes, times=262144):
result(dst aligned, src aligned): memcpy_fast=37638ms memcpy=23609 ms
result(dst aligned, src unalign): memcpy_fast=39622ms memcpy=19791 ms
result(dst unalign, src aligned): memcpy_fast=39921ms memcpy=32271 ms
result(dst unalign, src unalign): memcpy_fast=39559ms memcpy=34470 ms

benchmark(size=1048576 bytes, times=2048):
result(dst aligned, src aligned): memcpy_fast=52125ms memcpy=41194 ms
result(dst aligned, src unalign): memcpy_fast=78614ms memcpy=41211 ms
result(dst unalign, src aligned): memcpy_fast=78948ms memcpy=77637 ms
result(dst unalign, src unalign): memcpy_fast=49238ms memcpy=75619 ms

benchmark(size=4194304 bytes, times=512):
result(dst aligned, src aligned): memcpy_fast=79270ms memcpy=75565 ms
result(dst aligned, src unalign): memcpy_fast=77294ms memcpy=104157 ms
result(dst unalign, src aligned): memcpy_fast=101559ms memcpy=78571 ms
result(dst unalign, src unalign): memcpy_fast=103164ms memcpy=82106 ms

benchmark(size=8388608 bytes, times=256):
result(dst aligned, src aligned): memcpy_fast=115543ms memcpy=111483 ms
result(dst aligned, src unalign): memcpy_fast=113605ms memcpy=79576 ms
result(dst unalign, src aligned): memcpy_fast=111091ms memcpy=95734 ms
result(dst unalign, src unalign): memcpy_fast=110179ms memcpy=89143 ms

benchmark random access:
memcpy_fast=870612ms memcpy=695958ms
*/
