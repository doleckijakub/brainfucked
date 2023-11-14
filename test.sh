set -xe

g++ main.c
./a.out test.bf > test.nasm
nasm -f elf64 test.nasm
ld test.o -o test
./test
