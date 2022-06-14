#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#include <immintrin.h>

#ifndef __AVX2__
#error AVX2 not supported
#endif 

static uint64_t r = 1;

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
	while (!is_sorted(a, len)) {
		shuffle(a, len);
		//print_arr(a, len);
	}
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

void fill_shuffles() {
	// Create a bunch of shuffles (8! of them, but we'll only use some of them)
	shuffles = aligned_alloc(64, 40320 * sizeof(__m256i) / sizeof(char));

	int idx = 0;
	int e[8] = {};

	// Lazy and dumb
#define LS(i) for (e[i] = 0; e[i] < 8; ++(e[i])) {
	LS(6)	LS(2)	LS(1)	LS(3)	LS(0)	LS(4)	LS(7)	LS(5)

	int f = 0;
	for (int i = 0; i < 8; ++i) {
		f |= (1 << e[i]); 
	}

	if (f == 0xff) {
		for (int i = 0; i < 8; ++i) {
			shuffles[idx++] = e[i];
		}
	}

#define LE }
	LE LE LE LE LE LE LE LE

#undef LE
#undef LS
}

inline __m256i get_8x32_shuffle() {
	return _mm256_load_si256(rand_range(1500) + (const __m256i*) shuffles);
}


void chad_bogosort(int* a, int len) {
	// len is 16
	
	__m256i mask_highest = _mm256_setr_epi32(0, -1, -1, -1, -1, -1, -1, -1);
	
	__m256i part1 = _mm256_load_si256((const __m256i*) a);	
	__m256i part2 = _mm256_load_si256(1 + (const __m256i*) a);
	__m256i shuffle = get_8x32_shuffle();
	__m256i shuffled1, shuffled2, p1sh, p1sorted, p2sh, p2sorted;

	while (1) {
		// This code bottlenecks HARD on port 5 as you might expect; it's almost entirely shuffles
		// shift left by 4 bytes 
		p1sh = _mm256_alignr_epi8(part1,
				_mm256_permute2x128_si256(part1, part1, _MM_SHUFFLE(0, 0, 2, 0)),
			       12);
		p1sorted = _mm256_and_si256(_mm256_cmpgt_epi32(p1sh, part1), mask_highest);

		if (_mm256_testz_si256(p1sorted, p1sorted)) {
			goto test_p2_sorted;	
		}

shuffle:
		// Perform two shuffles within each register, then interleave registers into two new registers
		shuffled1 = _mm256_permutevar8x32_epi32(part1, shuffle);
		shuffled2 = _mm256_permutevar8x32_epi32(part2, shuffle);

		shuffle = get_8x32_shuffle();

		// Interleave into original registers and enjoy
		part1 = _mm256_unpackhi_epi32(shuffled1, shuffled2);
		part2 = _mm256_unpacklo_epi32(shuffled2, shuffled1);

		continue;

test_p2_sorted: p2sh = _mm256_alignr_epi8(part2,
			_mm256_permute2x128_si256(part2, part2, _MM_SHUFFLE(0, 0, 2, 0)),
		       12);

		p2sh = _mm256_insert_epi32(p2sh, _mm256_extract_epi32(part1, 7), 0);
		p2sorted = _mm256_cmpgt_epi32(p2sh, part2);

		if (_mm256_testz_si256(p2sorted, p2sorted)) {
			// store and return

			_mm256_store_si256((__m256i*) a, part1);
			_mm256_store_si256(1 + (__m256i*) a, part2);
			return;
		}

		goto shuffle;
	}
}

struct timespec start, finish;
double elapsed;

int main() {
	int len = 16;

	clock_gettime(CLOCK_MONOTONIC, &start);
	
	fill_shuffles();

	clock_gettime(CLOCK_MONOTONIC, &finish);

	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

	printf("Filled shuffles: %fs\n", elapsed);

	_Alignas(64) int arr[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }; // }; 100, 1000, 10000, 100000, 1000000, 10000000 };
	shuffle(arr, len);


	clock_gettime(CLOCK_MONOTONIC, &start);
	chad_bogosort(arr, len);
	clock_gettime(CLOCK_MONOTONIC, &finish);

	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

	printf("\nCompleted bogosort: %fs\nArray:", elapsed);


	print_arr(arr, len);
}
