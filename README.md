
# PCL Compiler

A personal project to build a simple PCL (Personal Computing Language) compiler.
Currently the compiler is working as a C-Subset Compiler, however future plans are to extend to a custom language with unique(+useful) semantics.

All parts of the compiler will be written in C.

Current Language Goals:

    -> Simple Arithematic Expressions
    -> Variables with datatypes: int, float, char, bool
    -> Basic I/O Functionaity
    -> Control and conditional statements.



Current Status: 
Lexer working and completed (Open to changes)
Expression parsing completed and working to improve statement parsing.
Symbol Table structure is made, deciding on few design decisions before integrating for semantic analysis.

For output checking:
make dev 
