# BrainFuck Compiler

![](bfc.png)

A native Brainfuck compiler, written in ANSI C.

Part of a fuller love letter to ANSI C.

Designed to provide maximum execution speed with tiny executables, optimizing for both raw speed and number of instructions.

## Scope

While unfinished, modular and has the ability to support other architectures, optimizations and operating systems.

- [x] Custom & integrated backend.
- [ ] Cross compilation built-in.
  - [x] ELF.
  - [ ] PE(EXE).
  - [ ] Mach-O.
- [ ] Cross architecture.
  - [x] Intel/AMD family.
  - [ ] ARM
  - [ ] RISCv.
- [ ] Optimization.
  - [x] Architecture specific machine-code optimization for size and speed.
  - [ ] Vectorization (is it worth it enough though?).
  - [ ] Dead code elimination.
  - [x] NOP elimination.
- [x] Fast compilation.
