# high-perf-bogosort
High-performance bogosort of integers using AVX2.

There are two implementations: a straightforward one (that operates on any size), and a less straightforward one that operates on 16-entry arrays only.

The standard implementation uses a Fisher-Yates shuffle. The accelerated implementation does a pretty good, but not perfect shuffle, of the entries, and then tests whether they are sorted. Doing an actual Fisher-Yates shuffle would be quite a bit more complicated. Also, this is one of those tasks which actually would benefit hugely from AVX512 and its cross-lane shuffles (`vpermd zmm, zmm, zmm`), but I sadly do not own such a computer to develop or test that on.

### Timings

#### CPU 1
Macbook Pro
CPU: i7-6820HQ (base frequency: 2.70 GHz, turbo frequency: 3.60 GHz, four-core turbo frequency: 3.20 GHz)
[WikiChip entry](https://en.wikichip.org/wiki/intel/core_i7/i7-6820hq)

Standard bogosort:

12 non-zero elements

Accelerated bogosort:
```
Threads (AVX2 only): 8
Timer filled shuffles finished: 0.000037 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 0 finished: 0.011855 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 1 finished: 0.009317 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 2 finished: 0.008605 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 3 finished: 0.008937 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 4 finished: 0.012001 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 5 finished: 0.029881 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 6 finished: 0.248614 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 7 finished: 2.616380 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 8 finished: 25.353464 seconds
```

(CPU hit max temps really quickly; the computer has horrible cooling)
