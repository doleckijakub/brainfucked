# Brainfucked

## Brief

A language based on [brainfuck](https://wikipedia.org/wiki/Brainfuck).
My experiment on making a usable programming language out of this esolang.

## Capabilities

The compiler supports all the functionality of brainfuck, compiling into **static x86_64 ELF executables**.
Please note that the program is currently dependent on [nasm](https://nasm.us), but it is planned to be discarded in the future;

## Usage

```console
# cc main.c -o brainfucked
# ./brainfucked file.bf
```

or

```console
# ./test.sh file.bf
```

## Plans

[x] - Full brainfuck functionality
[ ] - Optimizations
[ ] - Writing executables directly (without an assembler)
[ ] - New instructions and control flow
[ ] - Self hosting
[ ] - More platform support
