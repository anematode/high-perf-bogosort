#define _GNU_SOURCE
#define __USE_GNU

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include <immintrin.h>
#include <x86intrin.h>

#ifndef __AVX2__
#error AVX2 not supported
#endif 

#ifdef __linux__
#include <sched.h>
#endif

#define MAX_THREADS 24
// Whether to attempt to force threads to certain cores to prevent switching
#define attempt_taskset 1

#ifndef __linux__
#if attempt_taskset
#warning Taskset does not work on non-Linux systems; attempt_taskset will be set to 0
#undef attempt_taskset
#define attempt_taskset 0
#endif
#endif

static uint64_t SEED = 42;

static uint64_t r = 3;
static int result[24]; // only 16 are actually used
pthread_mutex_t result_mutex;

static uint64_t total_iters[MAX_THREADS];
static volatile int complete;  // if true, all threads stop working

struct timespec last_cleared;
static int taskset_enabled;

void set_taskset_enabled(int e) {
	if (e && !attempt_taskset) {
		printf("Tried to enable taskset when there was no option to do so; please set attempt_taskset to 1 if on Linux");
		abort();
	}
	taskset_enabled = e;
}

void summarize_ts_enabled() {
	printf("Taskset is enabled: %s\n", taskset_enabled ? "yes" : "no");
}

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

double get_elapsed();

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

// Print victory
static int print_which_thread_found;

#ifdef __linux__
int core_count() {
	return sysconf(_SC_NPROCESSORS_ONLN);
}

void taskset_thread_self(int core_id) {
	if (core_id >= core_count()) {
		// oops
		complete = 1;
		abort();
	}

	// Get a taskset struct
	cpu_set_t s;
	CPU_ZERO(&s);
	CPU_SET(core_id, &s);

	pthread_setaffinity_np(pthread_self(),
		sizeof(cpu_set_t), &s);
}
#endif

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

#define SHIFT_COUNT 8

// Select a random (variable) offset from here. Should fit in L1d. Not used anymore tho
static __m256i shifts[SHIFT_COUNT];

// L1d is 32 kb, so we shouldn't ever get a cache miss
#define SHUFFLE_COUNT 1024

void fill_shuffles() {
	shuffles = aligned_alloc(64, SHUFFLE_COUNT * sizeof(__m256i) / sizeof(char));

	int e[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };	

	// Any consecutive three bits (offset by multiple of three) is a valid shuffle, packing 21 shuffles per 256-bit chunk
	// Not used anymore tho
	for (int offset = 0; offset < SHIFT_COUNT; ++offset) {
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

#define SHOW_CYCLES 0

void* avx2_bogosort(void* _thread_id) {
	int thread_id = _thread_id ? *(int*) _thread_id : 0;
	int* a = result;

#ifdef __linux__
#if attempt_taskset
	if (taskset_enabled)
		taskset_thread_self(thread_id); // one logical core per physical core
#endif
#endif

	__m256i shift_right = _mm256_setr_epi32(0, 0, 1, 2, 3, 4, 5, 6);         // shift right one 32-bit int  (gets optimized out)
	
	// Split into two registers
	__m256i part1 = _mm256_load_si256((const __m256i*) a);	
	__m256i part2 = _mm256_load_si256(1 + (const __m256i*) a);
	__m256i shuffle1 = get_8x32_shuffle(0);
	__m256i shuffle2 = get_8x32_shuffle(1);
	__m256i shuffle3 = get_8x32_shuffle(3);
	__m256i shuffle4 = get_8x32_shuffle(2);

	// tmp registers (many get reused)
	__m256i shuffled1, shuffled2, p1sh, p1sorted, p2sh, p2sorted, interleaved1, interleaved2;
	
	uint64_t iters = 0;

	// Ensure different seeds are used
	
	pthread_mutex_lock(&result_mutex);
	uint64_t r = SEED++;
	pthread_mutex_unlock(&result_mutex);

#if SHOW_CYCLES
	unsigned int _;
	uint64_t cyc_start;
       	cyc_start = __rdtscp(&_);
#endif

	while (!complete) {
		// Bottleneck is memory loads (vmovdqa, latency 10!!)
		++iters;

		// Perform two shuffles within each register and interleave them
		shuffled1 = _mm256_permutevar8x32_epi32(part1, shuffle3);
		shuffled2 = _mm256_permutevar8x32_epi32(part2, shuffle4);

		r = r * 3 + 250182; // pseudorandom 64-bit

		shuffle3 = get_8x32_shuffle(r % SHUFFLE_COUNT);
		shuffle4 = get_8x32_shuffle((r >> 12) % SHUFFLE_COUNT);

		interleaved1 = _mm256_unpackhi_epi32(shuffled1, shuffled2);
		interleaved2 = _mm256_unpacklo_epi32(shuffled2, shuffled1);

		shuffled1 = _mm256_permutevar8x32_epi32(interleaved1, shuffle1);
		shuffled2 = _mm256_permutevar8x32_epi32(interleaved2, shuffle2);
		
		shuffle1 = get_8x32_shuffle((r >> 36) % SHUFFLE_COUNT);
		shuffle2 = get_8x32_shuffle((r >> 54) % SHUFFLE_COUNT);	

		part1 = _mm256_unpackhi_epi32(shuffled1, shuffled2);
		part2 = _mm256_unpacklo_epi32(shuffled2, shuffled1);	
		
		// check sorted
		p1sh = _mm256_permutevar8x32_epi32(part1, shift_right);
		p1sorted = _mm256_cmpgt_epi32(p1sh, part1);

		if (!_mm256_testz_si256(p1sorted, p1sorted)) {
			continue;
		}

		p2sh = _mm256_permutevar8x32_epi32(part2, shift_right);
		
		p2sh = _mm256_insert_epi32(p2sh, _mm256_extract_epi32(part1, 7), 0);
		p2sorted = _mm256_cmpgt_epi32(p2sh, part2);

		if (!_mm256_testz_si256(p2sorted, p2sorted)) {
			continue;
		}

		// Found!!
		// Write result	
		pthread_mutex_lock(&result_mutex);
		_mm256_store_si256((__m256i*) a, part1);
		_mm256_store_si256(1 + (__m256i*) a, part2);

		complete = 1;
		
		if (print_which_thread_found)
			printf("Thread %i found!\n", thread_id);

		pthread_mutex_unlock(&result_mutex);
		break;
	}
	
	// store and return
#if SHOW_CYCLES
	uint64_t cyc_end = __rdtscp(&_);
	printf("Cyc: %llu\n", cyc_end - cyc_start);
#endif

	// Force storage (for counting and such)
	_mm256_store_si256(2 + (__m256i*) a, p2sorted);

	pthread_mutex_lock(&result_mutex);
	total_iters[thread_id] += iters;
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

#define MAX_TIMER_DEPTH 10

struct timespec start_a[MAX_TIMER_DEPTH];
static int timespec_i;
double elapsed;

void time_start() {
	clock_gettime(CLOCK_MONOTONIC, &start_a[timespec_i]);
	timespec_i++;
}

double grab_elapsed() {
	struct timespec finish;
	clock_gettime(CLOCK_MONOTONIC, &finish);
	struct timespec start = start_a[timespec_i - 1];

	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	
	return elapsed;
}

void time_end(const char* msg) {
	if (msg) {

	printf("Timer %s finished: %f seconds\n", msg, grab_elapsed());
	}
	timespec_i--;
}

// Multithread AVX2 bogosort
void run_avx2_bogosort(int thread_count) {
	complete = 0;

	pthread_t threads[MAX_THREADS];
	int thread_ids[MAX_THREADS];

	for (int i = 0; i < thread_count; ++i) {
		thread_ids[i] = i;
		pthread_create(threads + i, NULL, &avx2_bogosort, (void*) &thread_ids[i]);
	}

	for (int i = 0; i < thread_count; ++i) {
		pthread_join(threads[i], NULL);
	}
}

void print_header() {
	printf("\n\nNonzero elems,Iters,Time in s,One iter every ns,Average ms per case\n");
}

// Bogosort n nonzero elements with 16 - n zero elements from n = 0 to 9
void run_bogosort_nonzero(int trials, int min, int max) {	
	time_start();

	for (int nonzero = min; nonzero < max; ++nonzero) {
		time_start();
		clear_total_iters();	

		for (int i = 0; i < trials; ++i) {
			fill_nonzero_elems(nonzero);
			shuffle(result, 16);

			bogosort(result, 16);
		}

		double e = grab_elapsed();
		print_header();
	        printf("%i,%i,%f,%f,%f\n", nonzero, trials, e, e / sum_total_iters() * 1.0e9, e / trials * 1.0e3);
		time_end(NULL);
		summarize_total_iters();
	}

	time_end("bogosort_nonzero");
}

// Bogosort n elements from n = 0 to 9
void run_bogosort(int trials, int min, int max) {
	time_start();

	for (int len = min; len < max; ++len) {
		time_start();
		clear_total_iters();	

		for (int i = 0; i < trials; ++i) {
			fill_nonzero_elems(len);
			shuffle(result, len);

			bogosort(result, len);
		}

		double e = grab_elapsed();
		print_header();
	        printf("%i,%i,%f,%f,%f\n", len, trials, e, e / sum_total_iters() * 1.0e9, e / trials * 1.0e3);
		time_end(NULL);
		summarize_total_iters();
	}
        
	time_end("bogosort");
}

void run_avx2_bogosort_nonzero(int trials, int min, int max, int thread_count)  {
	time_start();

	printf("\n\n\n\nRunning accelerated bogosort_nonzero, %i trials, minimum count %i, maximum count %i, thread count %i\n", trials, min, max, thread_count);
	summarize_ts_enabled();

	for (int nonzero = min; nonzero < max; ++nonzero) {
		time_start();
		clear_total_iters();	

		for (int i = 0; i < trials; ++i) {
			// Negligible timings
			fill_nonzero_elems(nonzero);
			shuffle(result, 16);
			
			run_avx2_bogosort(thread_count);
		}

		double e = grab_elapsed();	
		printf("Nonzero elems,Iters,Time in s,One iter every ns,Average ms per case,Thread count\n");
	        printf("%i,%i,%f,%f,%f,%i\n", nonzero, trials, e, e / sum_total_iters() * 1.0e9, e / trials * 1.0e3, thread_count);
		time_end(NULL);
		summarize_total_iters();
	}
	
	time_end("accelerated bogosort_nonzero");
}

void run_full_avx2_bogosort(int num_threads) {
	const int elem_count = 16;

	time_start();
	clear_total_iters();

	printf("Running full %i-element accelerated bogosort\n", elem_count);
	print_which_thread_found = 1;
	summarize_ts_enabled();

	fill_nonzero_elems(elem_count);
	shuffle(result, 16);

	printf("Unsorted array: ");
	print_arr(result, 16);

	run_avx2_bogosort(num_threads);

	printf("Sorted array: ");
	print_arr(result, 16);

	summarize_total_iters();

	char o[100];
	sprintf(o, "accelerated bogosort %i elements", elem_count);
	time_end(o);
}

void single_threaded_iters() {
	int nonzero_elems = 9;

	fill_nonzero_elems(nonzero_elems);
	shuffle(result, 16);
	time_start();
	printf("Unsorted array: ");
	print_arr(result, 16);	

	printf("Timing single-threaded accelerated bogosort with %i nonzero elements\n", nonzero_elems);
	clear_total_iters();

	avx2_bogosort(NULL);

	printf("Sorted array: ");
	print_arr(result, 16);

	summarize_total_iters();
	time_end("accelerated bogosort 10 nonzero elements single threaded");
}

void standard_battery_non_accel() {
	clear_total_iters();
	run_bogosort(100, 0, 11);
	clear_total_iters();
	run_bogosort_nonzero(100, 0, 7);
	clear_total_iters();
	run_bogosort_nonzero(20, 7, 8);
	clear_total_iters();
}

void standard_battery() {
	time_start();

	standard_battery_non_accel();

	int thread_counts[] = { 1, 2, 4, 8, 4, 8, 4, 8, 12, 16, 24 };
	int trialss[] = { 100, 100, 100, 100, 20, 20, 1, 1, 3, 3, 3 };
	int max_cnts[] = { 8, 8, 9, 9, 10, 10, 11, 11, 11, 11, 11 };
	int min_cnts[] = { 0, 0, 0, 0, 9, 9, 10, 10, 10, 10, 10 }; 

	for (int i = 0; i < 8; ++i) {
		int th_count = thread_counts[i], max_cnt = max_cnts[i], min_cnt = min_cnts[i], trials = trialss[i];

		if (th_count > MAX_THREADS) {
			// Skip
			continue;
		}

		set_taskset_enabled(0);	
		run_avx2_bogosort_nonzero(trials, min_cnt, max_cnt, th_count);
		clear_total_iters();

#if attempt_taskset
		set_taskset_enabled(1);
		run_avx2_bogosort_nonzero(trials, min_cnt, max_cnt, th_count);
		clear_total_iters();
#endif
	}

	time_end("standard battery");
}

int main() {
	setvbuf(stdout, NULL, _IONBF, 0);

	printf("Max threads (accelerated only): %i\n", MAX_THREADS);
	printf("Compiled with taskset: %s\n", attempt_taskset ? "yes" : "no");

	time_start();
	fill_shuffles();
	time_end("filled shuffles");

#ifdef __linux__
	set_taskset_enabled(1);
#endif
	standard_battery_non_accel();
	run_full_avx2_bogosort(MAX_THREADS);
	run_full_avx2_bogosort(MAX_THREADS);
}
