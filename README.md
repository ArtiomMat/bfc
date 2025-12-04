# BrainFuck Compiler

![](bfc.png)

A native Brainfuck compiler, written in ANSI C.

Part of a fuller love letter to ANSI C.

Designed to provide maximum execution speed with tiny executables, optimizing for both raw speed and number of instructions.

## Scope

- [x] Custom & integrated backend.
- [ ] Cross compilation built-in.
  - [ ] ELF.
  - [ ] PE(EXE).
  - [ ] Mach-O.
- [ ] Cross architecture.
  - [ ] Intel/AMD family.
  - [ ] ARM
  - [ ] RISCv.
- [ ] Optimization.
  - [ ] Architecture specific machine-code optimization for size and speed.
  - [ ] Vectorization (is it worth it enough though?).
  - [ ] Dead code elimination.
  - [ ] NOP elimination.
- [ ] Fast compilation.

## TODO

- [ ] Remove `G_ERROR` use return codes.
- [ ] Implement optimizer stage.
