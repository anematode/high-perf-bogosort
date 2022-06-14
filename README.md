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

#### CPU 2

CPU: i5-8250U CPU @ 1.60GHz 

Standard bogosort
```
Timer 100 trials of standard bogosort, 16 elems, nonzero = 0 finished: 0.000059 seconds
Timer 100 trials of standard bogosort, 16 elems, nonzero = 1 finished: 0.000850 seconds
Timer 100 trials of standard bogosort, 16 elems, nonzero = 2 finished: 0.014175 seconds
Timer 100 trials of standard bogosort, 16 elems, nonzero = 3 finished: 0.073353 seconds
Timer 100 trials of standard bogosort, 16 elems, nonzero = 4 finished: 0.571039 seconds
Timer 100 trials of standard bogosort, 16 elems, nonzero = 5 finished: 6.323554 seconds
Timer 100 trials of standard bogosort, 16 elems, nonzero = 6 finished: 61.942236 seconds
```

```
Timer 100 trials of standard bogosort, n = 0 finished: 0.000015 seconds
Timer 100 trials of standard bogosort, n = 1 finished: 0.000797 seconds
Timer 100 trials of standard bogosort, n = 2 finished: 0.006723 seconds
Timer 100 trials of standard bogosort, n = 3 finished: 0.045431 seconds
Timer 100 trials of standard bogosort, n = 4 finished: 0.627189 seconds
Timer 100 trials of standard bogosort, n = 5 finished: 7.050931 seconds
Timer 100 trials of standard bogosort, n = 6 finished: 63.023842 seconds
```

Accelerated bogosort
```
Threads (AVX2 only): 8
Timer filled shuffles finished: 0.000295 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 0 finished: 0.042485 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 1 finished: 0.015236 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 2 finished: 0.013019 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 3 finished: 0.013547 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 4 finished: 0.017556 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 5 finished: 0.046975 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 6 finished: 0.400089 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 7 finished: 4.201651 seconds
Timer 100 trials of AVX2 bogosort, nonzero = 8 finished: 43.361431 seconds
```
