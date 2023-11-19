set -xe

gcc -g -Wall -Wextra -Werror main.c lexer.c codegen.c -o brainfucked
./brainfucked $1 -o a.nasm
nasm -f elf64 -g a.nasm -o a.o
ld a.o -o a.out
./a.out
