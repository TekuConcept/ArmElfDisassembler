#ifndef ARM_DECOMPILER_H
#define ARM_DECOMPILER_H

#include "elf_parser.h"

void decompile(struct object&);

/*
1. disassemble instructions
2. organize instructions into tokens
3. mark relocations in tokens
4. check instruction availability
5. group tokens into functions
6. generate linked nodes from branch instructions
7. collapse instructions into C-equivalent code
8: ? - optimize - maybe group common statements into local variables
*/

#endif
