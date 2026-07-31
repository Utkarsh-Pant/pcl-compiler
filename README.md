
# PCL Compiler

A personal project to build a simple PCL (Personal Computing Language) compiler.
Currently the compiler is working as a C-Subset Compiler, however future plans are to extend to a custom language with unique(+useful) semantics.

All parts of the compiler will be written in C.

Current Language Goals:

    -> Simple Arithematic Expressions and operator support
    -> Variables with datatypes: int, char
    -> Basic I/O Functionaity
    -> Control and conditional statements.



Current Status: 
Lexer working and completed (Open to changes)
Parser working and completed w/ grammer parsing.
Semantic Analysis and symbol table generation complete.
Working on TAC generation.

For output checking:
make dev 
