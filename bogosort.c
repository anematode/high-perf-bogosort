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

#define THREAD_COUNT 8
static uint64_t SEED = 1;

static uint64_t r = 3;
static int result[16];
pthread_mutex_t result_mutex;

static uint64_t total_iters[THREAD_COUNT];
static volatile int complete;  // if true, all threads stop working

static int print_which_thread_found;

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

// L1d is 32 kb, so we shouldn't ever get a cache miss
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

void* avx2_bogosort(void* _thread_id) {
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
		
		if (print_which_thread_found)
			printf("Thread %i found!\n", thread_id);

		pthread_mutex_unlock(&result_mutex);

		break;
	}

	pthread_mutex_lock(&result_mutex);
	total_iters[thread_id] = iters;
	pthread_mutex_unlock(&result_mutex);

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

struct timespec start, finish;
double elapsed;


void time_start() {
	clock_gettime(CLOCK_MONOTONIC, &start);
}

void time_end(const char* msg) {
	if (!msg) msg = "(unnamed)";

	clock_gettime(CLOCK_MONOTONIC, &finish);

	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	
	printf("Timer %s finished: %f seconds\n", msg, elapsed);
}

// Multithread AVX2 bogosort
void run_avx2_bogosort() {
	complete = 0;

	pthread_t threads[THREAD_COUNT];
	int thread_ids[THREAD_COUNT];

	for (int i = 0; i < THREAD_COUNT; ++i) {
		thread_ids[i] = i;
		pthread_create(threads + i, NULL, &avx2_bogosort, (void*) &thread_ids[i]);
	}

	for (int i = 0; i < THREAD_COUNT; ++i) {
		pthread_join(threads[i], NULL);
	}
}

// Bogosort n nonzero elements with 16 - n zero elements from n = 0 to 9
void run_bogosort_100_nonzero() {	
	char o[100];
	for (int nonzero = 0; nonzero < 9; ++nonzero) {
		fill_nonzero_elems(nonzero);
		shuffle(result, 16);

		for (int i = 0; i < 100; ++i) {
			bogosort(result, 16);
		}
	}
}

// Bogosort n elements from n = 0 to 9
void run_bogosort_100() {
	int trials = 100;
	char o[100];
	for (int len = 0; len < 9; ++len) {
		time_start();

		fill_nonzero_elems(len);
		shuffle(result, len);

		for (int i = 0; i < trials; ++i) {
			bogosort(result, 16);
		}

		sprintf(o, "%i trials of standard bogosort, n = %i", trials, len);
		time_end(o);
	}
}

void run_avx2_bogosort_100_nonzero()  {
	int trials = 100;
	char o[100];

	for (int nonzero = 0; nonzero < 9; ++nonzero) {
		time_start();

		for (int i = 0; i < trials; ++i) {
			// Negligible timings
			fill_nonzero_elems(nonzero);
			shuffle(result, 16);
			
			run_avx2_bogosort();
		}

		sprintf(o, "%i trials of AVX2 bogosort, nonzero = %i", trials, nonzero);
		time_end(o);
	}
}

void run_full_avx2_bogosort() {
	print_which_thread_found = 1;

	fill_nonzero_elems(9);
	shuffle(result, 16);

	printf("Unsorted array: ");
	print_arr(result, 16);

	run_avx2_bogosort();

	printf("Sorted array: ");
	print_arr(result, 16);

	uint64_t sum_iters = 0;
	for (int i = 0; i < THREAD_COUNT; ++i) {
		uint64_t iters = total_iters[i];

		sum_iters += iters;
		printf("Thread %i iters: %llu\n", i, iters);
	}

	printf("\nTotal iters: %llu\n", sum_iters);

}

int main() {
	printf("Threads (AVX2 only): %i\n", THREAD_COUNT);

	time_start();
	fill_shuffles();
	time_end("filled shuffles");

	run_avx2_bogosort_100_nonzero();
}
