# high-perf-bogosort
High-performance bogosort of integers using AVX2.

There are two implementations: a straightforward one (that operates on any size), and a less straightforward one that operates on 16-entry arrays only.

The standard implementation uses a Fisher-Yates shuffle. The accelerated implementation does a pretty good, but not perfect shuffle. Doing an actual perfect shuffle would be quite a bit more complicated. Also, this is one of those tasks which actually would benefit hugely from AVX512, but I sadly do not own such a computer.

## Commentary

As you would expect, the accelerated implementation handily beats the straightforward one, because:

- Vertical vector comparison is branchless and there is only one particularly important branch in the main loop, which is taken very infrequently (only when the first 8 elements are sorted)
- Everything is done with shuffle instructions. The full shuffle is done with 5 shuffle loads (that is, grabbing 5 random shuffles from memory) and 8 shuffle instructions. There is some instruction-level parallelism in the shuffle instructions. 

#### Standard bogosort

If we have a random event that happens with probability *p*, the average number of samples until it happens is *1/p*. Thus, in bogosort, the expected value of the number of times the array is shuffled and tested is equal to the number of permutations of *n* elements.

There are two entries below. The first entry is the most obvious bogosort algorithm on *n* elements: we shuffle the *n* elements, then check whether they're sorted; that will take *n!* iterations on average. The second entry ("16 elems") is a bogosort on *16* elements, in which *n* of those elements are nonzero, a dilution of a full 16-element bogosort.

We need some numbers, huh. How fast is standard bogosort per iteration (that is, per shuffle + sorted test)? For 16 elements, about **338 cycles** on my Macbook laptop.

#### Accelerated bogosort

The accelerated implementation can only sort exactly 16 elements. Each element is a 32-bit integer, and there are two 256-bit registers which contain the 16 elements: 8 elements in one register and 8 elements in the other. The code repeatedly shuffles them and checks whether they are sorted. The shuffling isn't quite perfect.:

- Out of 1024 predetermined, random possibilities, choose a new order for each register (one order per register).
- The registers are shuffled, but no data actually *crosses* the registers.
- The registers are interleaved. The higher 128-bit parts of each register is interleaved,  and the lower 128-bit parts of each register is interleaved (with each other).

Sorted status is checked by shifting the first register to the right and doing a vertical comparison. If that half is sorted, it tests the second half, and if that's also sorted, the search is over. Multithreading bogosort is easy: each thread has its own registers and RNG, but shares the same 1024 possible re-orderings. The first thread that finds the prize sets a global flag and writes the result to a static array.

On my Macbook Pro, with 16 distinct elements, accelerated bogosort averages **14 cycles** per shuffle + test for single-threaded performance, as measured by `rdtsc` with turbo turned off. The actual shuffling of the main two registers has a loop-carried dependency chain of length **10 cycles**, so that's essentially the lower bound. The code branches off of *vptest* (latency 6), which isn't great if the branch is hard to predict. But the branch only happens when the lower register is sorted, which is infrequent when there are only a few duplicates. Hyperthreaded at full load, the new bottleneck seems to be port 5 contention: there are 10 instructions competing for port 5, and thus the theoretical maximum (which is actually nearly achieved!) is 10 cycles per iteration.

#### Performance analysis (slightly outdated)

The following is the most important section, compiled down:

```asm
LBB11_4:                                ## =>This Inner Loop Header: Depth=1
	cmp	dword ptr [rip + _complete], 0
	jne	LBB11_7
## %bb.5:                               ##   in Loop: Header=BB11_4 Depth=1
	inc	r13
	vpermd	ymm1, ymm5, ymm7        ## 
	vpermd	ymm2, ymm8, ymm2
	lea	rbx, [rbx + 2*rbx]
	add	rbx, 250182
	mov	ecx, ebx
	and	ecx, 1023
	shl	rcx, 5
	vmovdqa	ymm5, ymmword ptr [rax + rcx]
	mov	rcx, rbx
	shr	rcx, 7
	and	ecx, 32736
	vmovdqa	ymm8, ymmword ptr [rax + rcx]
	vpunpckhdq	ymm3, ymm1, ymm2
	vpermd	ymm3, ymm4, ymm3	      
	vpunpckldq	ymm1, ymm2, ymm1        ## ymm1 = ymm2[0],ymm1[0],ymm2[1],ymm1[1],ymm2[4],ymm1[4],ymm2[5],ymm1[5]
	vpermd	ymm1, ymm6, ymm1
	mov	rcx, rbx
	shr	rcx, 31
	and	ecx, 32736
	vmovdqa	ymm4, ymmword ptr [rax + rcx]
	mov	rcx, rbx
	shr	rcx, 49
	and	ecx, -32
	vmovdqa	ymm6, ymmword ptr [rax + rcx]
	vpunpckhdq	ymm7, ymm3, ymm1        ## ymm7 = ymm3[2],ymm1[2],ymm3[3],ymm1[3],ymm3[6],ymm1[6],ymm3[7],ymm1[7]
	vpunpckldq	ymm2, ymm1, ymm3        ## ymm2 = ymm1[0],ymm3[0],ymm1[1],ymm3[1],ymm1[4],ymm3[4],ymm1[5],ymm3[5]
	vpermd	ymm1, ymm0, ymm7
	vpcmpgtd	ymm1, ymm1, ymm7
	vptest	ymm1, ymm1
	jne	LBB11_4
```

`_complete` is a flag for whether the computation has been finished by another thread. `rbx` contains a random variable generated by a basic linear congruential generator. `rax` contains a pointer to a list of 1024 shuffles. An analysis/visualization of the pipeline and critical path is given in `results/mca_analysis.txt` and is corroborated by measurements. The critical path is port 5 with the *vpermd* (latency 3, throughput 1) and *vpunpck(hl)dq* (latency 1, throughput 1) instructions; each instruction decodes to one µop on port 5. *vptest* also runs on port 5, but its latency is hidden in the single-thread analysis. The 5-to-8 cycle latency of `vmovdqa` from memory is totally hidden.

#### Hyperthreading

Eight threads is substantially faster than four threads:

```
Timeline view:
                    0123456789          
Index     0123456789          0123456789
               v vptest from previous loop
[-1,31]   =====eeeER     .    .    .       vptest     ymm1, ymm1

            v vpermd begins executing
[0,3]     .DeeeE---R.    .    .    .       vpermd     ymm1, ymm5, ymm7
[0,4]     .D=eeeE--R.    .    .    .       vpermd     ymm2, ymm8, ymm2
[0,10]    . D====eeeeeeeER    .    .       vmovdqa    ymm5, ymmword ptr [rax + rcx]
[0,14]    .  D===eeeeeeeER    .    .       vmovdqa    ymm8, ymmword ptr [rax + rcx]
               ^^ two wasted cycles; second cycle is wasted due to vptest in previous iteration
[0,15]    .   D==eE------R    .    .       vpunpckhdq ymm3, ymm1, ymm2
[0,16]    .   D===eeeE---R    .    .       vpermd     ymm3, ymm4, ymm3
[0,17]    .   D====eE----R    .    .       vpunpckldq ymm1, ymm2, ymm1
[0,18]    .   D=====eeeE-R    .    .       vpermd     ymm1, ymm6, ymm1
                     ^^ two wasted cycles
[0,27]    .    . D=====eE---R .    .       vpunpckhdq ymm7, ymm3, ymm1
[0,28]    .    . D======eE--R .    .       vpunpckldq ymm2, ymm1, ymm3
[0,29]    .    . D=======eeeER.    .       vpermd     ymm1, ymm0, ymm7
[0,30]    .    . D==========eER    .       vpcmpgtd   ymm1, ymm1, ymm7
[0,31]    .    .  D==========eeeER .       vptest     ymm1, ymm1
[0,32]    .    .  D=============eER.       jne        LBB11_4
                          ^^ no cycles wasted here because vptest only needs ymm1 and ymm1 is only written to for the next few cycles
[1,0]     .    .   DeeeeeeE-------R.       cmp        dword ptr [rip + _complete], 0
[1,1]     .    .   D======eE------R.       jne        LBB11_7
[1,3]     .    .    D=====eeeE----R.       vpermd     ymm1, ymm5, ymm7
                          ^ next vpermd begins executing
...
```

There are precisely four wasted cycles on port 5 here. In other words, port 5 is not starting an instruction for four cycles out of every fourteen. When hyperthreading, two threads share the same CPU core and are thus able to issue instructions at twice the rate, but the execution units are not duplicated. Thus, there are no more wasted cycles (on each one, the *other* thread will issue an instruction to port 5. The cycles per loop per thread go up to 20, but the overall performance is improved by a factor of 28%. This behavior is borne out by the data—hyperthreading seems to speed it up to 10.2 cycles per iteration.

## Timings

How to run:
```
./build.sh
./bogosort | tee results/xxx.txt
```

### CPU 1
Macbook Pro
CPU: Core i7-8559U (4 cores, base clock: 2.7GHz; max turbo: 4.5GHz; Coffee Lake)
[WikiChip entry](https://en.wikichip.org/wiki/intel/core_i7/i7-8559u)

For four- and eight-threaded test cases, the CPU quickly throttles to 77–80% of its turbo frequency due to thermal issues. For the single- and two-threaded cases, it seems to stay at 100%. These are the figures I use for estimations.

### CPU 2
Linux laptop
CPU: i5-8250U (4 cores, base frequency: 1.6 GHz; max turbo: 3.4 GHz; Kaby Lake)
[WikiChip entry](https://en.wikichip.org/wiki/intel/core_i5/i5-8250u)

For four- and eight-threaded test cases, the CPU quickly throttles to 2400 GHz or so (70% of the turbo frequency). It does not seem to throttle for single- and two-threaded cases. Thankfully, `perf stat` gives thorough details on 

### CPU 3
2020 Mac Pro
CPU: Xeon W-3235 (12 cores, base clock: 3.4 GHz; max turbo: 4.4 GHz; Cascade Lake)
[WikiChip entry](https://en.wikichip.org/wiki/intel/xeon_w/w-3235)
