gcc -S bogosort.c -o results/bogosort.s -O2 -mllvm --x86-asm-syntax=intel -march=native -pthread -Wall

 /usr/local/opt/llvm/bin/llvm-mca results/critical_path.s -mcpu=skylake  --iterations=800 -timeline -dispatch 4 -o results/mca_analysis.txt
