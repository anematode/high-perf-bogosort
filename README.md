# high-perf-bogosort
High-performance bogosort of integers using AVX2.

There are two implementations: a straightforward one (that operates on any size), and a less straightforward one that operates on 16-entry arrays only.

The standard implementation uses a Fisher-Yates shuffle. The accelerated implementation does a pretty good, but not perfect shuffle, of the entries, and then tests whether they are sorted. Doing an actual Fisher-Yates shuffle would be quite a bit more complicated. Also, this is one of those tasks which actually would benefit hugely from AVX512 and its shuffles (`vpermd zmm, zmm, zmm`, since the array would fit a zmm register), but I sadly do not own such a computer.

## Commentary

As you would expect, the accelerated implementation handily beats the straightforward one. A few reasons:

- Vertical vector comparison is branchless and there is only one particularly important branch in the main loop, which is taken very infrequently (only when the first 8 elements are sorted)
- Everything is done with shuffles (in the instruction sense; x86 has a lot of shuffle instructions). The shuffle is done with 5 shuffle loads (that is, grabbing 5 random shuffles from memory) and 8 shuffle instructions. There is some instruction-level parallelism in the shuffle instructions. 

One computer is of course much faster than the other. The Linux laptop, however, has much better cooling; the Macbook quickly hit 95 degrees and throttled (not good for it!).

#### Standard bogosort

A simple mathematical preliminary. If we have a random event that happens with probability *p*, the average number of samples until it  happens is *1/p*. Thus, in our case, the average number of *shuffles* (i.e., the number of times the array is shuffled and tested whether it's sorted) is just the total number of shuffles. (Note each shuffle is statistically independent by bogosort's wise design.)

There are two entries below. The first entry is the most obvious bogosort algorithm on *n* elements: we shuffle the *n* elements, then check whether they're sorted. Great. That will take *n!* iterations on average. The second entry ("16 elems") is a bogosort on *16* elements, where *n* of those elements are nonzero. If you think about it, a lot of the tested shuffles will look like `0, 1, 0, 2, 3, 4, 0, 0, 5, 0, 6, 0, 0, 7, 8, 9`, in which the nonzero elements are in order but the shuffle is rejected because they're not all in order. That can be done in *16! x nCr(16, n)* ways, which is much, much greater than *n!* for *n <= 15*. And, as we observe, the second implementation takes much longer. Alas, they eventually both get too slow to measure without dying of boredom.

Even a bogosort enjoyer like myself would not implement the second entry for no reason. It is only implemented because it is logically equivalent to the AVX2 implementation.

We need some numbers, huh. How fast is standard bogosort per iteration (that is, per shuffle + sorted test)? For 16 elements, about **338 cycles** on my Macbook Pro.

#### Accelerated bogosort

The accelerated implementation, fundamentally, can only sort exactly 16 elements. Each element is a 32-bit integer, and there are two 256-bit *registers* (loosely, high-speed-access pieces of memory) which contain the 16 elements: 8 elements in one register and 8 elements in the other. The code repeatedly shuffles them, then checks whether they are sorted. The shuffling isn't quite a Fisher-Yates shuffle; the algorithm is as follows:

- Out of 1024 predetermined, random possibilities, choose a new order for each register (one order per register). Note that there are actually *8! = 40320* orders, but a smaller number makes it much faster (it fits in L1d cache easily)
- The registers are shuffled, but no data actually *crosses* the registers.
- The registers are interleaved. The higher 128-bit parts of each register is interleaved,  and the lower 128-bit parts of each register is interleaved (with each other). 

There is a lot of shuffling here. I chose this system because it actually produces a fairly random shuffle, which I feel is the ethos of bogosort. If you think about it, the "amount" of randomness "introduced" is about 1024^5 = 1.1e15 > 2e13 = 16!, so, although it's not perfect, it gives you a pretty darn random result. Meanwhile, in previous implementations, I only had a lackluster shuffling algorithm (shuffle both registers with the *same* random order, then interleave them--a meager 1024 possibilities). When I ran these implementations, I observed that the number of iterations they had was less than the theoretical bogosort. Why? Because the shuffles were no longer statistically independent, and so fewer trials were necessary. Imagine spending hours writing an AVX2 implementation of your favorite algorithm, learning about cross-lane shuffling, trying to find the most performant way to check for sorting (`vpermd` ultimately works better than `vpalignr`)... only to find that your algorithm is not faithful to bogosort, that your shuffles are not statistically independent. Oh, the disgust, and the bathos of it all!

More seriously, sorted status is checked by shifting the first register to the right and doing a vertical comparison. If it's all 0s then that half is sorted, and it jumps to a (not particularly optimized) test for whether the second half is sorted. If so, the search is over! Multithreading bogosort is easy: each thread has its own registers and RNG, but shares the same 1024 possible orders. The first thread that finds the prize sets a global flag, writes the result to a static array, and terminates in glory. The rest of the threads, seeing that the work is done, terminate equally gloriously--in bogosort, unlike in the Olympics, there are participation trophies.

On my Macbook Pro, with 16 distinct elements, accelerated bogosort averages **14 cycles** per shuffle + test for single-threaded performance, as measured by `rdtsc` with turbo turned off. The measured amount agrees with llvm-mca analysis of the critical path. The bottleneck is vector loads of shuffles, which could be optimized better for sure. I'll probably get to it once I'm better with optimization. Ignoring the test for sorting, the actual shuffling of the main two registers has a loop-carried dependency chain of length **10 cycles**, so that's essentially the lower bound. The code branches off of *vptest* (latency 6), which isn't great if the branch is hard to predict. But the branch only happens when the lower register is sorted, which is infrequent when there are only a few duplicates. Hyperthreaded at full load, the new bottleneck seems to be port 5 contention: there are 10 instructions competing for port 5, and thus the theoretical maximum (which is actually nearly achieved!) is 10 cycles per iteration.

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

`_complete` is a flag for whether the computation has been finished by another thread. `rbx` contains a random variable generated by a basic linear congruential generator. `rax` contains a pointer to a list of 1024 shuffles. An analysis/visualization of the pipeline and critical path is given in `results/mca_analysis.txt` and is corroborated by measurements. The critical path is port 5 with the *vpermd* (latency 3, throughput 1) and *vpunpck(hl)dq* (latency 1, throughput 1) instructions; each instruction decodes to one µop on port 5. *vptest* also runs on port 5, but its latency is hidden in the single-thread analysis. The 5-to-8 cycle latency of `vmovdqa` is totally hidden.

#### Hyperthreading

Eight threads is substantially faster than four threads, and it's not too difficult to see why if you look at the critical path. Observe (noting that *vptest* is one uop but is hidden because the branch is extremely predictable):

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

There are precisely four wasted cycles on port 5 (read: execution unit 5) here. In other words, port 5 is not starting an instruction for four cycles out of every fourteen. (Note that, as a hard rule, any given port can only *begin* executing one instruction per cycle. It can, however, *continue* to execute multiple instructions in parallel.) When hyperthreading, two threads share the same CPU core and are thus able to issue instructions at twice the rate, but the execution units are not duplicated. Thus, these wasted cycles are eliminated (on each one, the *other* thread will issue an instruction to port 5. Now, an instruction can be issued on port 5 every cycle, and the cycles per loop per physical core is 10 (the number of ops which go to port 5). Of course, the cycles per loop per thread go up to 20, but the overall performance is improved by a factor of 28%, which is nonnegligible. This behavior is borne out by the data--hyperthreading seems to speed it up to 10.2 cycles per iteration.

It turns out that macOS has a CPU affinity system too, although not as well known as Linux's. Alas, it is somewhat complicated.

## Timings

How to run:
```
./build.sh
./bogosort | tee results/xxx.txt
```

If you do decide to run this, make sure your computer doesn't kill itself!

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

## Miscellaneous commands

Perf counting:

```
sudo perf stat -e uops_dispatched_port.port_0 -e uops_dispatched_port.port_1 -e uops_dispatched_port.port_2 -e uops_dispatched_port.port_3 -e uops_dispatched_port.port_4 -e uops_dispatched_port.port_5 -e uops_dispatched_port.port_6 -e uops_dispatched_port.port_7 -e cpu-cycles -a ./bogosort
```


```
sudo perf stat -a ./bogosort
```

Check macOS thermal throttling:
```
pmset -g thermlog
```


Output ASM on GCC:
```
gcc -S bogosort.c -O2 -march=native -o bin/bogosort.s -masm=intel
```

Emulate AVX-512:
```
/usr/local/bin/intel-sde/sde64 -- bin/bogosort_avx512 -- user-application
```
