set -xe

g++ main.c -o brainfucked
./brainfucked $1 > test.nasm
nasm -f elf64 -g test.nasm
ld test.o -o test
./test
