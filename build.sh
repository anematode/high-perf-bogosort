mkdir bin
cp bogosort.c bin/bogosort.c
cd bin
gcc bogosort.c -o bogosort -O2 -march=native -pthread -g -Wall
cd ..
