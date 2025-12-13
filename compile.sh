#!/bin/bash
./bfc $1 
objdump -D -b binary -m i386:x86-64:intel bfcbin
as wrapper.s -o wrapper.o && ld wrapper.o -o bfcbin.elf
