# PCL (Personal Computing Language)

PCL is a custom, from-scratch compiler written entirely in C. It translates a C-like procedural language into raw x86-64 Linux assembly (System V AMD64 ABI). 

Built as an exploration of low-level systems engineering, PCL manages its own end-to-end compilation pipeline without relying on external frontend frameworks like LLVM or Bison/Flex.

## Architecture & Compilation Pipeline

PCL is designed with a clean, multi-stage architecture:

1. **Lexical Analysis:** Tokenizes raw source code into a predictable stream.
2. **LL(1) Parsing:** Constructs a robust Abstract Syntax Tree (AST), gracefully desugaring complex structures (e.g., converting `for` loops into `while` blocks).
3. **Semantic Analysis:** A scope-aware type checker that enforces initialization rules, verifies operator compatibility, and manages stack boundaries.
4. **Intermediate Representation (IR):** Translates the AST into Three-Address Code (TAC), flattening nested expressions and handling temporary variable allocation.
5. **x86-64 Backend:** Generates valid, executable Intel-syntax assembly, correctly mapping TAC instructions to hardware registers and the stack.

## Current Features

* **Control Flow:** `if`, `while`, and `for` loop constructs.
* **Memory Management:** Scope-aware stack allocation with high-water mark tracking for nested blocks.
* **System V ABI Function Calls:** Support for external function calls, including variadic functions like `printf` and `scanf`.
* **String Literals:** Automatic pooling and extraction of strings into the `.rodata` segment.
* **Basic Operators:** Full suite of arithmetic, relational, and basic address-of (`&`) operators.

## Usage Example

PCL currently compiles complex control flows and external calls. Here is a working Fibonacci sequence generator in PCL:

```c
int a = 0;
int b = 1;
int n = 0;

printf("Enter the value of n:");
scanf("%d", &n);

"Random String For Test Since Dont Have Optimization Lol";

printf("Fibonnaci series upto %d is:\n", n);
for(int i = 0; i<n; i++){
	int c = a + b;
	a = b;
	b = c;
	&a;

	printf("%d ", a);
}

printf("\n");
```

## Building and Running

Ensure you have `gcc` and `make` installed on your Linux system.
The Makefile contains all instructions for full-compilation.
For a testing enviornment, just edit test/target.pcl program and run `make dev`

## Roadmap

The compiler is currently in an active prototype phase. Upcoming milestones include:

- [ ] **Codebase Refactoring**  
  Clean up early prototype logic and eliminate abstraction leaks.

- [ ] **Robust Type System**  
  Full implementation of explicit pointer types (`KWD_PTR`) and dynamic sizing, removing current `TAC_ADDR` hacks.

- [ ] **Module System**  
  Support for file imports and multi-file compilation.

- [ ] **Optimization Passes**  
  Introduce basic TAC optimization passes, including:
  - Constant folding
  - Dead code elimination
  - Additional pre-assembly optimizations

---

## ‍Author

Developed by **Utkarsh Pant** :) 




