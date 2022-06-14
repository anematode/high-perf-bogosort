# high-perf-bogosort
High-performance bogosort using AVX2.

The


### Timings

CPU: i7-6820HQ (base frequency: 2.70 GHz, turbo frequency: 3.60 GHz)
[WikiChip entry](https://en.wikichip.org/wiki/intel/core_i7/i7-6820hq)




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

