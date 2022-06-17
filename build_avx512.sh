# Target: cascade lake
# Have to run with Intel SDE because I don't have an AVX-512-supporting computer

mkdir bin
cp bogosort.c bin/bogosort.c
cd bin
gcc bogosort.c -o bogosort_avx512 -O2 -march=cascadelake -pthread -g -Wall
cd ..

