#define _GNU_SOURCE
#define __USE_GNU

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <immintrin.h>
#include <x86intrin.h>

#ifndef __AVX2__
#error AVX2 not supported
#endif 

static uint64_t SEED = 1;

static uint64_t r = 3;
static int result[16];

#define MAX_THREADS 1
static uint64_t total_iters[MAX_THREADS];

struct timespec last_cleared;

uint64_t sum_total_iters() {
	uint64_t total = 0;
	for (int i = 0; i < MAX_THREADS; ++i)
		total += total_iters[i];

	return total;
}

void clear_total_iters() {
	memset(total_iters, 0, MAX_THREADS * sizeof(uint64_t));
	clock_gettime(CLOCK_MONOTONIC, &last_cleared);
}

void summarize_total_iters() {
	uint64_t sum_iters = 0;
	for (int i = 0; i < MAX_THREADS; ++i) {
		uint64_t iters = total_iters[i];

		sum_iters += iters;
		if (iters) printf("Thread %i iters: %llu\n", i, iters);
	}

	printf("\nTotal iters: %llu\n", sum_iters);
	
	struct timespec finish;
	clock_gettime(CLOCK_MONOTONIC, &finish);

	double elapsed = (finish.tv_sec - last_cleared.tv_sec);
	elapsed += (finish.tv_nsec - last_cleared.tv_nsec) / 1000000000.0;
	
	printf("One iter every ns: %f\n", elapsed / sum_iters * 1.0e9);
}

int rand_range(int max) {
	r = r * 3 + 1034011;

	return r % max;
}

int is_sorted(int* a, int len) { 
	for (int i = 0; i < len - 1; ++i) {
		if (a[i] > a[i+1]) return 0;
	}
	return  1;
}

inline void shuffle(int* a, int len) {
	for (int i = len - 1; i >= 1; --i) {
		int j = rand_range(i + 1);
		int tmp = a[i];
		a[i] = a[j];
		a[j] = tmp;
	}
}

void print_arr(int* a, int len) {
	for (int i = 0; i < len; ++i) {
		printf("%i ", a[i]);
	}
	printf("\n");
}

void bogosort(int* a, int len) {
	uint64_t iters = 0;

	while (!is_sorted(a, len)) {
		shuffle(a, len);
		iters++;
	}

	total_iters[0] += iters;
}

// Credit: https://stackoverflow.com/a/49154279/13458117
void log_mm256(const __m256i value)
{
    const size_t n = sizeof(__m256i) / sizeof(int);
    int buffer[n];

    _mm256_storeu_si256((__m256i*)buffer, value);

    for (int i = 0; i < n; i++)
        printf("%i ", buffer[i]);
    printf("\n");
}

static int* shuffles;

// Select a random (variable) offset from here. Should fit in L1d
static __m256i shifts[10];

static int complete;

// L1d is 32 kb, so we shouldn't ever get a cache miss
#define SHUFFLE_COUNT 1024

void fill_shuffles() {
	shuffles = aligned_alloc(64, SHUFFLE_COUNT * sizeof(__m256i) / sizeof(char));

	int e[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };	

	// Any consecutive three bits (offset by multiple of three) is a valid shuffle, packing 21 shuffles per 256-bit chunk
	for (int offset = 0; offset < 10; ++offset) {
		int idx = 0;
		for (int i = 0; i < SHUFFLE_COUNT; ++i) {
			shuffle(e, 8);

			for (int j = 0; j < 8; ++j)
				shuffles[idx++] |= e[j] << (offset * 3);
		}

		_mm256_store_si256(shifts + offset, _mm256_set1_epi32(3 * offset));
	}
}

inline __m256i get_8x32_shuffle(int idx) {
	return _mm256_load_si256(idx + (const __m256i*) shuffles);
}

inline __m256i get_shuffle_shift(int idx) {
	return _mm256_load_si256(idx + shifts);
}

#define SHOW_CYCLES 1

void* avx2_bogosort(void* _thread_id) {
	int thread_id = _thread_id ? *(int*) _thread_id : 0;
	int* a = result;

	__m256i mask_highest = _mm256_setr_epi32(0, -1, -1, -1, -1, -1, -1, -1); // extract highest
	__m256i shift_right = _mm256_setr_epi32(0, 0, 1, 2, 3, 4, 5, 6);         // shift right one 32-bit int  (gets optimized out)
	
	// Split into two registers
	__m256i part1 = _mm256_load_si256((const __m256i*) a);	
	__m256i part2 = _mm256_load_si256(1 + (const __m256i*) a);
	__m256i shuffle1 = get_8x32_shuffle(0);
	__m256i shuffle2 = get_8x32_shuffle(1);

	// tmp registers
	__m256i shuffled1, shuffled2, p1sh, p1sorted, p2sh, p2sorted, interleaved1, interleaved2, shuffle_shift;

	uint64_t iters = 0;

	// Ensure different seeds are used
	
	uint64_t r = SEED++;

	unsigned int _;
	uint64_t cyc_start;
       	cyc_start = __rdtscp(&_);

	while (iters < 100000000) {	
		// Bottleneck is throughput on port 5 (vpermd, vpunpckhdq, vpunpckldq), 10 instructions with TP 1 -> 10 cycles
		++iters;

		// Perform two shuffles within each register and interleave them
		shuffled1 = _mm256_permutevar8x32_epi32(part1, shuffle1);
		shuffled2 = _mm256_permutevar8x32_epi32(part2, shuffle2);
		
		r = r * 3 + 250182; // pseudorandom 64-bit

		part1 = _mm256_unpackhi_epi32(shuffled1, shuffled2);
		part2 = _mm256_unpacklo_epi32(shuffled2, shuffled1);
		continue;

		shuffled1 = _mm256_permutevar8x32_epi32(interleaved1, shuffle1);
		shuffled2 = _mm256_permutevar8x32_epi32(interleaved2, shuffle2);
	
		part1 = _mm256_unpackhi_epi32(shuffled1, shuffled2);
		part2 = _mm256_unpacklo_epi32(shuffled2, shuffled1);	

		// check sorted
		p1sh = _mm256_permutevar8x32_epi32(part1, shift_right);
		// Compiles to vpblendd
		p1sorted = _mm256_and_si256(_mm256_cmpgt_epi32(p1sh, part1), mask_highest);

		// End main loop early
		continue;
	
		if (!_mm256_testz_si256(p1sorted, p1sorted)) {
			continue;
		}

		// Compiles to a vpalignr instruction (but not on the critical path, so nbd)
		p2sh = _mm256_permutevar8x32_epi32(part2, shift_right);
		
		p2sh = _mm256_insert_epi32(p2sh, _mm256_extract_epi32(part1, 7), 0);
		p2sorted = _mm256_cmpgt_epi32(p2sh, part2);

		if (!_mm256_testz_si256(p2sorted, p2sorted)) {
			continue;
		}
	
		// store and return
		_mm256_store_si256((__m256i*) a, part1);
		_mm256_store_si256(1 + (__m256i*) a, part2);

		complete = 1;
		
		break;
	}
	
	// Force storage
	uint64_t cyc_end = __rdtscp(&_);
	printf("Cyc: %llu\n", cyc_end - cyc_start);
	_mm256_store_si256((__m256i*) a, part1);

	total_iters[thread_id] += iters;

	return NULL;
}

void fill_nonzero_elems(int nonzero_elems) {
	for (int i = 0; i < nonzero_elems; ++i) {
		result[i] = i + 1;
	}

	for (int i = nonzero_elems; i < 16; ++i) {
		result[i] = 0;
	}
}

void single_threaded_iters() {
	fill_nonzero_elems(10);
	shuffle(result, 16);

	printf("Timing single-threaded accelerated bogosort with 10 nonzero elements\n");
	clear_total_iters();

	avx2_bogosort(NULL);

	summarize_total_iters();
}

int main() {
	fill_shuffles();

	single_threaded_iters();
}
