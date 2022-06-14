#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include <immintrin.h>

#ifndef __AVX2__
#error AVX2 not supported
#endif 

#define THREAD_COUNT 4
static uint64_t SEED = 1;

static uint64_t r = 3;
static int result[16];
pthread_mutex_t result_mutex;

static uint64_t total_iters[THREAD_COUNT];
static volatile int complete;  // if true, all threads stop working

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

	total_iters[0] = iters;
}

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

// L1d is 48000 bytes, so we shouldn't ever get a cache miss
#define SHUFFLE_COUNT 1024

void fill_shuffles() {
	shuffles = aligned_alloc(64, SHUFFLE_COUNT * sizeof(__m256i) / sizeof(char));

	int idx = 0;
	int e[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };	

	for (int i = 0; i < SHUFFLE_COUNT; ++i) {
		shuffle(e, 8);

		for (int j = 0; j < 8; ++j)
			shuffles[idx++] = e[j];
	}
}

inline __m256i get_8x32_shuffle(int idx) {
	return _mm256_load_si256(idx + (const __m256i*) shuffles);
}

void* chad_bogosort(void* _thread_id) {
	int thread_id = *(int*) _thread_id;
	int* a = result;

	__m256i mask_highest = _mm256_setr_epi32(0, -1, -1, -1, -1, -1, -1, -1); // extract highest
	__m256i shift_right = _mm256_setr_epi32(0, 0, 1, 2, 3, 4, 5, 6);         // shift right one 32-bit int
	
	// Split into two registers
	__m256i part1 = _mm256_load_si256((const __m256i*) a);	
	__m256i part2 = _mm256_load_si256(1 + (const __m256i*) a);
	__m256i shuffle1 = get_8x32_shuffle(0);
	__m256i shuffle2 = get_8x32_shuffle(1);

	// tmp registers
	__m256i shuffled1, shuffled2, shuffle3, p1sh, p1sorted, p2sh, p2sorted, interleaved1, interleaved2;

	uint64_t iters = 0;

	// Ensure different seeds are used
	
	pthread_mutex_lock(&result_mutex);
	uint64_t r = SEED++;
	pthread_mutex_unlock(&result_mutex);

	while (!complete) {
		// This code probably bottlenecks HARD on port 5
		
		++iters;

		// Perform two shuffles within each register and interleave them
		shuffled1 = _mm256_permutevar8x32_epi32(part1, shuffle1);
		shuffled2 = _mm256_permutevar8x32_epi32(part2, shuffle2);
		
		r = r * 3 + 250182; // pseudorandom 64-bit

		shuffle1 = get_8x32_shuffle(r % SHUFFLE_COUNT);
		shuffle2 = get_8x32_shuffle((r >> 12) % SHUFFLE_COUNT);
		shuffle3 = get_8x32_shuffle((r >> 24) % SHUFFLE_COUNT);

		// Shuffle a shuffle for some extra randomness
		shuffle1 = _mm256_permutevar8x32_epi32(shuffle1, shuffle3);

		interleaved1 = _mm256_unpackhi_epi32(shuffled1, shuffled2);
		interleaved2 = _mm256_unpacklo_epi32(shuffled2, shuffled1);

		// Do it again
		shuffle1 = get_8x32_shuffle((r >> 36) % SHUFFLE_COUNT);
		shuffle2 = get_8x32_shuffle((r >> 48) % SHUFFLE_COUNT);

		shuffled1 = _mm256_permutevar8x32_epi32(part1, shuffle1);
		shuffled2 = _mm256_permutevar8x32_epi32(part2, shuffle2);

		part1 = _mm256_unpackhi_epi32(shuffled1, shuffled2);
		part2 = _mm256_unpacklo_epi32(shuffled2, shuffled1);

		// check sorted
		p1sh = _mm256_permutevar8x32_epi32(part1, shift_right);
		p1sorted = _mm256_and_si256(_mm256_cmpgt_epi32(p1sh, part1), mask_highest);

		if (!_mm256_testz_si256(p1sorted, p1sorted)) {
			continue;
		}

		p2sh = _mm256_permutevar8x32_epi32(part2, shift_right);
		
		p2sh = _mm256_insert_epi32(p2sh, _mm256_extract_epi32(part1, 7), 0);
		p2sorted = _mm256_cmpgt_epi32(p2sh, part2);

		if (!_mm256_testz_si256(p2sorted, p2sorted)) {
			continue;
		}
	
		// store and return
		pthread_mutex_lock(&result_mutex);
		_mm256_store_si256((__m256i*) a, part1);
		_mm256_store_si256(1 + (__m256i*) a, part2);

		complete = 1;
		
		printf("Thread %i found!\n", thread_id);
		pthread_mutex_unlock(&result_mutex);

		break;
	}

	pthread_mutex_lock(&result_mutex);
	total_iters[thread_id] = iters;
	pthread_mutex_unlock(&result_mutex);

	return NULL;
}

struct timespec start, finish;
double elapsed;

void run_avx2_bogosort() {
	pthread_t threads[THREAD_COUNT];
	int thread_ids[THREAD_COUNT];

	for (int i = 0; i < THREAD_COUNT; ++i) {
		thread_ids[i] = i;
		pthread_create(threads + i, NULL, &chad_bogosort, (void*) &thread_ids[i]);
	}

	clock_gettime(CLOCK_MONOTONIC, &start);
	for (int i = 0; i < THREAD_COUNT; ++i) {
		pthread_join(threads[i], NULL);
	}
}

int main() {
	_Alignas(64) int arr[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 0, 0, 0, 0 };
	memcpy(result, arr, sizeof(int) * 16);

	int len = 16; // unused for accelerated bogosort

	clock_gettime(CLOCK_MONOTONIC, &start);
	
	fill_shuffles();

	clock_gettime(CLOCK_MONOTONIC, &finish);

	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

	printf("Filled shuffles: %fs\n", elapsed);

	shuffle(result, 16);

	printf("Array: ");
	print_arr(result, 16);

	run_avx2_bogosort();

	clock_gettime(CLOCK_MONOTONIC, &finish);

	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

	printf("\nCompleted bogosort: %fs\nArray: ", elapsed);
	print_arr(result, len);

	uint64_t sum_iters = 0;
	for (int i = 0; i < THREAD_COUNT; ++i) {
		uint64_t iters = total_iters[i];

		sum_iters += iters;
		printf("Thread %i iters: %llu\n", i, iters);
	}

	printf("\nTotal iters: %llu\n", sum_iters);

	// Wait for input
	int _;
	scanf("%i", &_);
}
