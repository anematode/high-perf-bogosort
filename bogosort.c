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
#error AVX2 not supported (boring!)
#endif 

#ifdef __linux__
#include <sched.h>
#endif

// Change this to your desired thread count
#define MAX_THREADS 8
// Whether to attempt to force threads to certain cores to prevent switching
#define attempt_taskset 1
// Whether to attempt avx512 (automatically disabled if not supported)
#define attempt_avx512 1
// Print cycle count for avx2 thingy
#define SHOW_CYCLES 0

/**
 * Determines the program's random number generator. By design, the program is deterministic (in the sense that
 * if you run it twice, the unsorted arrays will always be exactly the same 
 */
static uint64_t SEED = 202;

static uint64_t r = 3;
static int result[24]; // only 16 are actually used
pthread_mutex_t result_mutex;

// Whether to print victory
static int print_which_thread_found;

static uint64_t total_iters[MAX_THREADS];
// Note: rdtsc runs at a constant rate, independent of clock and context switches, so it has limited utility
static uint64_t measured_rdtsc[MAX_THREADS]; 
// if true, all threads immediately terminate. Note that this is really undefined behavior, but in practice it works
// with the volatile kw. Don't do this in general!
static volatile int complete;

struct timespec last_cleared;
// Whether to run taskset
static int taskset_enabled;

#ifndef __linux__
#if attempt_taskset
#warning Taskset does not work on non-Linux systems; attempt_taskset will be set to 0
#undef attempt_taskset
#define attempt_taskset 0
#endif
#endif

#ifdef __AVX512F__
#pragma message ("Computer supports AVX512")
#else

#if attempt_avx512
#warning AVX512 is not supported on this system (turning off)
#undef attempt_avx512
#define attempt_avx512 0
#endif
#endif

void set_taskset_enabled(int e) {
	if (e && !attempt_taskset) {
		printf("Tried to enable taskset when there was no option to do so; please set attempt_taskset to 1 if on Linux\n");
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
	memset(measured_rdtsc, 0, MAX_THREADS * sizeof(uint64_t));
	clock_gettime(CLOCK_MONOTONIC, &last_cleared);
}

double get_elapsed();

void summarize_total_iters() {
	uint64_t sum_iters = 0;
	uint64_t sum_rdtsc = 0;

	for (int i = 0; i < MAX_THREADS; ++i) {
		uint64_t iters = total_iters[i];
		uint64_t rdtsc = measured_rdtsc[i];

		sum_iters += iters;
		sum_rdtsc += rdtsc;

		if (iters) printf("Thread %i iters: %llu\n", i, iters);
		if (rdtsc) printf("Thread %i rdtsc: %llu\n", i, rdtsc);
	}

	printf("\nTotal iters: %llu\n", sum_iters);
	printf("\nTotal rdtsc: %llu\n", sum_rdtsc);
	
	struct timespec finish;
	clock_gettime(CLOCK_MONOTONIC, &finish);

	double elapsed = (finish.tv_sec - last_cleared.tv_sec);
	elapsed += (finish.tv_nsec - last_cleared.tv_nsec) / 1000000000.0;
	
	printf("One iter every ns: %f\n", elapsed / sum_iters * 1.0e9);
}

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
	r = r * 6018048192831 + 1;

	return (r >> 9) % max;
}

int is_sorted(int* a, int len) { 
	for (int i = 0; i < len - 1; ++i) {
		if (a[i] > a[i+1]) return 0;
	}
	return  1;
}

// Fisher-Yates
void shuffle(int* a, int len) {
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

void standard_bogosort(int* a, int len) {
	uint64_t iters = 0;

	while (!is_sorted(a, len)) {
		shuffle(a, len);
		iters++;
	}

	total_iters[0] += iters;
}

__m128i* shuffle_dump;
uint64_t shuffle_dump_idx;
uint64_t shuffle_dump_cap = 0;

// Dumps a shuffle into two 64-bit chunks by converting to bytes from 32-bit integers
void dump_shuffle(const __m256i part1, const __m256i part2) {
	if (shuffle_dump_idx >= shuffle_dump_cap) {
		shuffle_dump = realloc(shuffle_dump,
				sizeof(__m128i) * (shuffle_dump_cap = shuffle_dump_cap * 1.5 + 1000));
	}


	const __m256i pack_shuffle = _mm256_setr_epi8(
			0, 4, 8, 12, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, 0, 4, 8, 12, 
			-1, -1, -1, -1, -1, -1, -1, -1);
	const __m256i pack_shuffle2 = _mm256_setr_epi8(
			-1, -1, -1, -1, -1, -1, -1, -1,
			0, 4, 8, 12, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, 0, 4, 8, 12);

	__m256i pack1 = _mm256_shuffle_epi8(part1, pack_shuffle);
	__m256i pack2 = _mm256_shuffle_epi8(part2, pack_shuffle2);

	__m128i compressed = _mm_or_si128(
			_mm_or_si128(
				_mm_or_si128(
					_mm256_castsi256_si128(pack1),
					_mm256_castsi256_si128(pack2)),
				_mm256_extracti128_si256(pack1, 1)),
			_mm256_extracti128_si256(pack2, 1));

	_mm_storeu_si128(&shuffle_dump[shuffle_dump_idx++], compressed);
}

int cmp_shuffles(const __m128i shuf1, const __m128i shuf2) {
	// Lexicographically compare two shuffles
	int gt = _mm_movemask_epi8(_mm_cmpgt_epi8(shuf1, shuf2));
	int lt = _mm_movemask_epi8(_mm_cmpgt_epi8(shuf2, shuf1));

	return lt - gt;	 // if equal, the shuffles are equal, etc
}

void insertion_sort_shuffles(__m128i* start, uint64_t len) {
	for (size_t i = 1; i < len; ++i) {
		__m128i key = _mm_loadu_si128(&start[i]);

		__m128i poop;

		size_t j = i;
		while ((j > 0) && cmp_shuffles(key, poop = _mm_loadu_si128(&start[j - 1])) < 0) {
			_mm_storeu_si128(&start[j], poop);
			--j;
		}
		_mm_storeu_si128(&start[j], key);
	}
}

void quicksort_shuffles(__m128i* start, uint64_t len) {
	if (len < 26) {
		insertion_sort_shuffles(start, len);
		return;
	}

  __m128i pivot = _mm_loadu_si128(&start[len / 2]);

  uint64_t i, j;
  for (i = 0, j = len - 1; ; i++, j--) {
    while (0 < cmp_shuffles(pivot, _mm_loadu_si128(start + i))) i++;
    while (0 > cmp_shuffles(pivot, _mm_loadu_si128(start + j))) j--;

    if (i >= j) break;

    __m128i temp_i = _mm_loadu_si128(start + i);
    __m128i temp_j = _mm_loadu_si128(start + j);

    _mm_storeu_si128(start + i, temp_j);
    _mm_storeu_si128(start + j, temp_i);
  }

  quicksort_shuffles(start, i);
  quicksort_shuffles(start + i, len - i);
}

void sort_shuffles(__m128i* entries, uint64_t count) {
	quicksort_shuffles(entries, count);
}

void log_mm128(const __m128i value)
{
    const size_t n = sizeof(__m128i) / sizeof(uint8_t);
    uint8_t buffer[n];

    _mm_storeu_si128((__m128i*)buffer, value);

    for (int i = 0; i < n; i++)
        printf("%i ", buffer[i]);
    printf("\n");
}

void analyze_shuffles(__m128i* entries, uint64_t count) {
	// For statistical analysis; entries are compactified into 128-bit groupings of bytes
	
	printf("Analyzing randomness\n");

	sort_shuffles(entries, shuffle_dump_idx);
	int duplicates[16] = { 0 };
	int same = 0;

	printf("Sorted\n");

	for (int i = 1; i < shuffle_dump_idx; ++i) {
		__m128i s1 = _mm_loadu_si128(&shuffle_dump[i - 1]);
		__m128i s2 = _mm_loadu_si128(&shuffle_dump[i]);

		__m128i neq = _mm_xor_si128(s1, s2);
		if (_mm_testz_si128(neq, neq)) {
			// Same
			same++;
			if (same >= 16) same = 15;
		} else {
			duplicates[same]++;
			same = 0;
		}
	}

	for (int i = 0; i < 16; ++i) {
		printf("Duplicates of count %i: %llu\n", i, duplicates[i]);
	}
	printf("Total shuffles: %llu\n", shuffle_dump_idx);
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

#ifdef __AVX_512__
void log_mm512(const __m512i value)
{
    const size_t n = sizeof(__m512i) / sizeof(int);
    int buffer[n];

    _mm512_storeu_si512((__m512i*)buffer, value);

    for (int i = 0; i < n; i++)
        printf("%i ", buffer[i]);
    printf("\n");
}
#endif
static int* shuffles;

#define SHIFT_COUNT 8

// Select a random (variable) offset from here. Should fit in L1d. Not used anymore tho
static __m256i shifts[SHIFT_COUNT];

#define SHUFFLE_COUNT 2048

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

__m256i get_8x32_shuffle(int idx) {
	return _mm256_load_si256(idx + (const __m256i*) shuffles);
}

__m256i get_shuffle_shift(int idx) {
	return _mm256_load_si256(idx + shifts);
}

#define COMMON_INIT_G  \
	int thread_id = _thread_id ? *(int*) _thread_id : 0; \
	int* a = result; \
	uint64_t iters = 0; \
	/* Ensure different seeds are used */ \
	pthread_mutex_lock(&result_mutex); \
	uint64_t r = SEED++; \
	pthread_mutex_unlock(&result_mutex); 

#if SHOW_CYCLES
#undef COMMON_INIT_CYCLES
#define COMMON_INIT_CYCLES \
	unsigned int _; \
	uint64_t cyc_start; \
       	cyc_start = __rdtscp(&_);
#else
#define COMMON_INIT_CYCLES
#endif

#ifdef __linux__ 
#if attempt_taskset
#undef COMMON_INIT_TASKSET
#define COMMON_INIT_TASKSET  \
	if (taskset_enabled) \
		taskset_thread_self(thread_id); /* one logical core per physical core */ 
#endif 
#else
#define COMMON_INIT_TASKSET ;
#endif

#define COMMON_INIT COMMON_INIT_G COMMON_INIT_TASKSET COMMON_INIT_CYCLES

#if SHOW_CYCLES
#define STORE_CYCLES_COMMON \
	uint64_t cyc_end = __rdtscp(&_); \
	measured_rdtsc[thread_id] += cyc_end - cyc_start;
#else
#define STORE_CYCLES_COMMON ;
#endif

#ifdef __AVX512F__
// We'll assume an L1d of 32 kib
#define AVX512_SHUFFLE_COUNT 65536

__m512i* sh;

pthread_mutex_t avx512ld_mutex;

void init_avx512_shuffles() {
	_Alignas(64) static int bleh[16];
	for (int i = 0; i < 16; ++i) {
		bleh[i] = i;
	}

	__m512i inc = _mm512_set1_epi32(4);

	for (int i = 0; i < AVX512_SHUFFLE_COUNT; ++i) {
		__m512i k = _mm512_setzero_si512();
		__m512i sl_count = _mm512_setzero_si512();

		for (int j = 0; j < 16; ++j) {
			shuffle(bleh, 16);

			__m512i sh = _mm512_load_si512((const __m512i*) bleh);

			k = _mm512_or_si512(k, _mm512_sllv_epi32(sh, sl_count));
			sl_count = _mm512_add_epi32(sl_count, inc);
		}

		_mm512_store_si512(&sh[i], k);
	}
}

__m512i* alloc_avx512_shuffles() {
	pthread_mutex_lock(&avx512ld_mutex);
		sh = aligned_alloc(64, AVX512_SHUFFLE_COUNT * sizeof(__m512i) / sizeof(char));

		init_avx512_shuffles();
	pthread_mutex_unlock(&avx512ld_mutex);

	return sh;
}

void* avx512_bogosort(void* _thread_id) {
	COMMON_INIT
	
	__m512i* sh = alloc_avx512_shuffles();

	__m512i arr = _mm512_load_si512((const __m512i*) a);
	__m512i next_arr;
	__m512i shuffle_list = _mm512_load_si512(sh);
	__m512i shuffle_list2, shuffle_list3, shuffle_list4;

	__m512i swapped_arr;
	__m512i shift_right = _mm512_setr_epi32(0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);

	__mmask16 every_other = 0x5555; // mask every other integer
	__mmask16 cmp;

	// Write result
	while (!complete) {
		if (r % (1 << 10) == 0) {
			alloc_avx512_shuffles();
		}
		// There are really only 8192 distinct 16-element shuffles we can choose immediately at random,
		// and only 1024 test orders. With so few shuffles, it's easy to have cycles etc. that make it
		// far less random. The solution is not actually too hard: Immediately after using the shuffle,
		// we shuffle the shuffle and store it back (the exact algorithm is a bit involved). We try to
		// hide the latency of the shuffles with test instructions.

		r = 8121 * r + 28411;

#define ROLL_SHUFFLE shuffle_list = _mm512_rol_epi32(shuffle_list, 4);
#define APPLY_SHUFFLE arr = _mm512_permutexvar_epi32(shuffle_list, arr); 
#define JMP_POSSIBLY_SORTED \
		swapped_arr = _mm512_rol_epi64(arr, 32); \
		cmp = _mm512_mask_cmpgt_epi32_mask(every_other, arr, swapped_arr); \
		iters++; \
		if (_ktestz_mask16_u8(cmp, cmp)) goto test_sorted;
#define ITER APPLY_SHUFFLE JMP_POSSIBLY_SORTED ROLL_SHUFFLE

		ITER ITER
		ITER ITER
		ITER ITER
		ITER ITER

#undef ITER	
sort_fail:
		shuffle_list = _mm512_load_si512(&sh[r % AVX512_SHUFFLE_COUNT]);
		r = 3 * r + 25018;
		shuffle_list2 = _mm512_load_si512(&sh[r % AVX512_SHUFFLE_COUNT]);

		shuffle_list = _mm512_permutexvar_epi32(shuffle_list, shuffle_list2);

		continue;
test_sorted:
		swapped_arr = _mm512_permutexvar_epi32(shift_right, arr);
		cmp = _mm512_cmpgt_epi32_mask(swapped_arr, arr);

		log_mm512(arr);
		if (!_ktestz_mask64_u8(cmp, cmp)) goto sort_fail;

		// Found
		pthread_mutex_lock(&result_mutex);

		if (complete)
			break;
		_mm512_store_si512((__m512i*) a, arr);

		complete = 1;
		
		if (print_which_thread_found)
			printf("Thread %i found!\n", thread_id);

		pthread_mutex_unlock(&result_mutex);
		break;
	}

	// store and return
	STORE_CYCLES_COMMON

	pthread_mutex_lock(&result_mutex);
	total_iters[thread_id] += iters;
	pthread_mutex_unlock(&result_mutex);

	return NULL;
}
#else
void* avx512_bogosort(void* _thread_id) {
	abort();
}
#endif /* __AXV512F__ */

uint64_t next_r(uint64_t r) {
	return 6364136223846793005ULL * r + 1;
}

#define DUMP_SHUFFLES 0

void* avx2_bogosort(void* _thread_id) {
	COMMON_INIT
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

	r = next_r(r);

	while (!complete) {
		iters++;

		// Perform two shuffles within each register and interleave them
		shuffled1 = _mm256_permutevar8x32_epi32(part1, shuffle3);
		shuffled2 = _mm256_permutevar8x32_epi32(part2, shuffle4);

		r = next_r(r); // pseudorandom 64-bit

		shuffle3 = get_8x32_shuffle((r >> 5) % SHUFFLE_COUNT);
		shuffle4 = get_8x32_shuffle((r >> 53) % SHUFFLE_COUNT);

		interleaved1 = _mm256_unpackhi_epi32(shuffled1, shuffled2);
		interleaved2 = _mm256_unpacklo_epi32(shuffled2, shuffled1);

		part1 = interleaved1;
		part2 = interleaved2;
		// part1 = _mm256_permutevar8x32_epi32(interleaved1, shuffle1);
		// part2 = _mm256_permutevar8x32_epi32(interleaved2, shuffle2);
		
		shuffle1 = get_8x32_shuffle((r >> 32) % SHUFFLE_COUNT);
		shuffle2 = get_8x32_shuffle((r >> 54) % SHUFFLE_COUNT);

		// Somewhat better shuffled, but ultimately not by enough to justify using 2 extra uops
		// part1 = _mm256_unpackhi_epi32(shuffled1, shuffled2);
		// part2 = _mm256_unpacklo_epi32(shuffled2, shuffled1);	
#if DUMP_SHUFFLES
		dump_shuffle(part1, part2);
#endif // DUMP_SHUFFLES
	
		// check sorted
		p1sh = _mm256_permutevar8x32_epi32(part1, shift_right);
		p1sorted = _mm256_cmpgt_epi32(p1sh, part1);

		// Better than vptest (saves a uop on port 5)
		__m256 p1sorted_f = _mm256_castsi256_ps(p1sorted);
	
		if (__builtin_expect(!_mm256_testz_ps(p1sorted_f, p1sorted_f), 1)) { // (_mm256_testz_si256(p1sorted, p1sorted))
			continue;
		}

		p2sh = _mm256_permutevar8x32_epi32(part2, shift_right);
		
		p2sh = _mm256_insert_epi32(p2sh, _mm256_extract_epi32(part1, 7), 0);
		p2sorted = _mm256_cmpgt_epi32(p2sh, part2);

		__m256 p2sorted_f = _mm256_castsi256_ps(p2sorted);

		if (__builtin_expect(!_mm256_testz_ps(p2sorted_f, p2sorted_f), 1)) {
			continue;
		}

		// Found
		// Write result	
		pthread_mutex_lock(&result_mutex);

		if (complete) break;
		_mm256_store_si256((__m256i*) a, part1);
		_mm256_store_si256(1 + (__m256i*) a, part2);

		complete = 1;
		
		if (print_which_thread_found)
			printf("Thread %i found!\n", thread_id);

		pthread_mutex_unlock(&result_mutex);
		break;
	}
	
	// store and return
	STORE_CYCLES_COMMON

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
void run_accel_bogosort(int thread_count, int use_avx512) {
	complete = 0;

	pthread_t threads[MAX_THREADS];
	int thread_ids[MAX_THREADS];

	// For each other thread, make one
	for (int i = 1; i < thread_count; ++i) {
		thread_ids[i] = i;
		pthread_create(threads + i, NULL, use_avx512 ? &avx512_bogosort : &avx2_bogosort, (void*) &thread_ids[i]);
	}

	// Make the current thread do work
	if (use_avx512)
		avx512_bogosort(0);
	else
		avx2_bogosort(0);	
	
	// Join the other threads
	for (int i = 1; i < thread_count; ++i) {
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

			standard_bogosort(result, 16);
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

			standard_bogosort(result, len);
		}

		double e = grab_elapsed();
		print_header();
	        printf("%i,%i,%f,%f,%f\n", len, trials, e, e / sum_total_iters() * 1.0e9, e / trials * 1.0e3);
		time_end(NULL);
		summarize_total_iters();
	}
        
	time_end("bogosort");
}

// void run_general_nonzero(int trials, int min, int max, int tc, void (*impl) 

void run_accel_bogosort_nonzero(int trials, int min, int max, int thread_count, int use_avx512)  {
	time_start();

	printf("\n\n\n\nRunning accelerated bogosort_nonzero, %i trials, minimum count %i, maximum count %i, thread count %i, AVX-512 %i\n", trials, min, max, thread_count, use_avx512);
	summarize_ts_enabled();

	for (int nonzero = min; nonzero < max; ++nonzero) {
		time_start();
		clear_total_iters();	

		for (int i = 0; i < trials; ++i) {
			// Negligible timings
			fill_nonzero_elems(nonzero);
			shuffle(result, 16);
				
			run_accel_bogosort(thread_count, use_avx512);
		}

		double e = grab_elapsed();	
		printf("Nonzero elems,Iters,Time in s,One iter every ns,Average ms per case,Thread count\n");
	        printf("%i,%i,%f,%f,%f,%i\n", nonzero, trials, e, e / sum_total_iters() * 1.0e9, e / trials * 1.0e3, thread_count);
		time_end(NULL);
		summarize_total_iters();
	}
	
	time_end("accelerated bogosort nonzero");
}

void run_full_accel_bogosort(int num_threads) {
	const int elem_count = 10;

	time_start();
	clear_total_iters();

	printf("Running full %i-element accelerated bogosort, %i threads (AVX-512: %i)\n", elem_count, num_threads, attempt_avx512);
	print_which_thread_found = 1;
	summarize_ts_enabled();

	fill_nonzero_elems(elem_count);
	shuffle(result, 16);

	printf("Unsorted array: ");
	print_arr(result, 16);

	run_accel_bogosort(num_threads, attempt_avx512);

	printf("Sorted array: ");
	print_arr(result, 16);

	summarize_total_iters();

	char o[100];
	sprintf(o, "Accelerated bogosort %i elements", elem_count);
	time_end(o);
}

void single_threaded_iters() {
	int nonzero_elems = 9;

	fill_nonzero_elems(nonzero_elems);
	shuffle(result, 16);
	time_start();
	printf("Unsorted array: ");
	print_arr(result, 16);	

	printf("Timing single-threaded accelerated bogosort with %i nonzero elements (AVX-512: %i)\n", nonzero_elems, attempt_avx512);
	clear_total_iters();

	if (attempt_avx512)
		avx512_bogosort(NULL);
	else
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
		run_accel_bogosort_nonzero(trials, min_cnt, max_cnt, th_count, 1);
		clear_total_iters();

#if attempt_taskset
		set_taskset_enabled(1);
		run_accel_bogosort_nonzero(trials, min_cnt, max_cnt, th_count, 1);
		clear_total_iters();
#endif
#if attempt_avx512
		run_accel_bogosort_nonzero(trials, min_cnt, max_cnt, th_count, 1);
		clear_total_iters();
#endif
	}

	time_end("standard battery");
}

void analyze_statistical_power() {
	shuffle_dump_idx = 0;
	run_full_accel_bogosort(1);
	analyze_shuffles(shuffle_dump, shuffle_dump_idx);
}

int main() {
	SEED *= 50;
	setvbuf(stdout, NULL, _IONBF, 0);

	printf("Max threads (accelerated only): %i\n", MAX_THREADS);
	printf("Shuffle lookup table size: %i\n", SHUFFLE_COUNT);
	printf("Compiled with taskset: %s\n", attempt_taskset ? "yes" : "no");

	time_start();
	fill_shuffles();
	time_end("filled shuffles");

#ifdef __linux__
	set_taskset_enabled(1);
#endif

	// analyze_statistical_power();
	run_full_accel_bogosort(1);
	// run_accel_bogosort_nonzero(100, 9, 10, 8, 0);
}

