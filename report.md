# Mini-Compiler for C-like Language — Final Report

## 1. Design Choices & Architecture Overview

The compiler follows a traditional multi-phase architecture:

```
Source Code → Lexer → Token Stream → Parser → AST → Semantic Analyzer → Code Generator → TAC
```

Each phase is independent and communicates through well-defined interfaces. The pipeline is orchestrated by the driver (`index.cpp`), which calls each phase sequentially and reports errors through a centralized `ErrorHandler`.

### Architecture Diagram

```
               +-----------+
               | Source    |
               | File      |
               +-----+-----+
                     |
                     v
               +-----------+
               | Lexer     |  Tokenizes char-by-char
               +-----+-----+
                     | Token Stream
                     v
               +-----------+
               | Parser    |  Recursive descent, builds AST
               +-----+-----+
                     | AST (Program*)
                     v
          +----------+----------+
          |                     |
          v                     v
   +-----------+        +-------------+
   | Symbol    |        | Semantic    |
   | Table     |<------>| Analyzer    |
   +-----------+        +------+------+
                               | Annotated AST
                               v
                      +-------------+
                      | Code Gen    |  Three Address Code
                      +------+------+
                             | TAC Instructions
                             v
                      +-------------+
                      | Output      |
                      +-------------+
```

### Key Design Decisions

| Decision | Rationale |
|----------|-----------|
| Hand-crafted lexer/parser | Full control, no dependencies, educational value |
| Recursive descent parser | Matches BNF grammar rules 1:1, easy to debug |
| AST with explicit node types | Type safety, clean visitor pattern |
| Linked-list statements | Simple chaining without dynamic arrays per block |
| TAC as intermediate representation | Close to machine code, easy to generate and debug |
| Centralized ErrorHandler | Collects all errors across phases, enables recovery |

## 2. BNF Grammar

```
Program       → { FunctionDecl }

FunctionDecl  → Type IDENTIFIER ( ParamList ) BlockStmt
ParamList     → ε | Param { , Param }
Param         → Type IDENTIFIER
Type          → int | float | bool | void

BlockStmt     → { { Stmt } }
Stmt          → DeclStmt | AssignStmt | IfStmt | WhileStmt
              | ForStmt | ReturnStmt | InputStmt | OutputStmt
              | BlockStmt

DeclStmt      → Type IDENTIFIER [ = Expr ] ;
AssignStmt    → IDENTIFIER = Expr ;
IfStmt        → if ( Expr ) Stmt [ else Stmt ]
WhileStmt     → while ( Expr ) Stmt
ForStmt       → for ( [DeclStmt | AssignStmt] ; [Expr] ; [AssignStmt] ) Stmt
ReturnStmt    → return [Expr] ;
InputStmt     → input ( IDENTIFIER ) ;
OutputStmt    → output ( Expr ) ;

Expr          → OrExpr
OrExpr        → AndExpr { || AndExpr }
AndExpr       → EqExpr { && EqExpr }
EqExpr        → RelExpr [ (== | !=) RelExpr ]
RelExpr       → AddExpr [ (< | > | <= | >=) AddExpr ]
AddExpr       → MulExpr { (+ | -) MulExpr }
MulExpr       → UnaryExpr { (* | /) UnaryExpr }
UnaryExpr     → [ ! | - ] PrimaryExpr
PrimaryExpr   → IDENTIFIER | INTEGER | FLOAT | true | false
              | ( Expr ) | FunCallExpr
FunCallExpr   → IDENTIFIER ( ArgList )
ArgList       → ε | Expr { , Expr }
```

## 3. Implementation Details per Phase

### Phase 1: Lexical Analyzer (Lexer)

**Files:** `include/Lexer.h`, `src/Lexer.cpp`

The lexer reads the source character-by-character and produces a stream of tokens. It handles:

- **Identifiers & Keywords:** Reads alphanumeric + underscore sequences, maps keywords via string comparison
- **Numbers:** Integers (`[0-9]+`) and floats (`[0-9]+.[0-9]+`)
- **Operators:** Single-character (`+`, `-`, `*`, `/`) and multi-character (`==`, `!=`, `<=`, `>=`, `&&`, `||`)
- **Delimiters:** `(`, `)`, `{`, `}`, `;`, `,`
- **Comments:** Single-line (`//`) and multi-line (`/* */`) with proper nesting-ignorant skip
- **Whitespace:** Spaces, tabs, newlines skipped

**Token Types:** 36 distinct types defined in the `TokenType` enum, including keywords (`INT`, `FLOAT`, `BOOL`, `IF`, etc.), operators (`PLUS`, `EQ`, `AND`, etc.), literals (`INT_VAL`, `FLOAT_VAL`, `ID`), and special tokens (`END_OF_FILE`, `ILLEGAL`).

### Phase 2: Abstract Syntax Tree (AST)

**Files:** `include/AST.h`

The AST uses a class hierarchy with a base `ASTNode` class. Key node types:

- **Expressions:** `IntLiteral`, `FloatLiteral`, `BoolLiteral`, `Identifier`, `UnaryExpr`, `BinaryExpr`, `FunCallExpr`
- **Statements:** `DeclStmt`, `AssignStmt`, `IfStmt`, `WhileStmt`, `ForStmt`, `ReturnStmt`, `InputStmt`, `OutputStmt`, `BlockStmt`
- **Structure:** `FunctionDecl` (with params, return type, body), `Program` (top-level container)

Statements are linked via `Stmt::next` pointer (linked-list pattern). Each parent node owns its children through pointer semantics, with destructors handling recursive cleanup.

### Phase 3: Syntax Analyzer (Parser)

**Files:** `include/Parser.h`, `src/Parser.cpp`

Recursive descent parser with one function per grammar rule. The parser:

- Accepts a vector of tokens from the lexer
- Returns a `Program*` AST root
- Implements panic-mode error recovery: on syntax error, skips tokens until `;` or `}` is found
- Reports errors via the `ErrorHandler` with line:column context

**Key parsing functions:**

| Function | Grammar Rule |
|----------|-------------|
| `parseProgram` | Program → { FunctionDecl } |
| `parseFunctionDecl` | FunctionDecl → Type ID ( ParamList ) BlockStmt |
| `parseStmt` | Stmt → (dispatches to specific statement parsers) |
| `parseExpr` → `parseOrExpr` | Expression precedence climbing |
| `parseFunCallExpr` | FunCallExpr → ID ( ArgList ) |

### Phase 4: Symbol Table

**Files:** `include/SymbolTable.h`, `src/SymbolTable.cpp`

Implements nested scoping using a stack of `map<string, Symbol>`:

- `pushScope()` / `popScope()` — enters/exits a block scope
- `insert()` — adds a symbol to the current scope, detects redeclaration
- `lookup()` — searches all scopes from innermost to outermost (shadowing)
- `lookupCurrentScope()` — checks only the current scope

Each `Symbol` stores: name, type (`INT`, `FLOAT`, `BOOL`, `VOID`), declaration line, and scope level.

### Phase 5: Semantic Analyzer

**Files:** `include/SemanticAnalyzer.h`, `src/SemanticAnalyzer.cpp`

Walks the AST and performs type checking using the symbol table:

- **Variable declaration check:** Ensures variables are declared before use via `symTable.lookup()`
- **Redeclaration check:** Uses `symTable.lookupCurrentScope()` before inserting
- **Type compatibility:** `canAssign(dest, src)` checks numeric-to-numeric coercion (int↔float), rejects bool mixing
- **Return type matching:** Uses `currentReturnType` tracking per function
- **Condition types:** If/while conditions must be boolean or integer
- **Operator type checking:** Arithmetic requires numeric operands, logical requires boolean operands

### Phase 6: Error Handler

**Files:** `include/ErrorHandler.h`

Centralized error collection with:

- `addError(line, col, phase, message)` — stores an error
- `printErrors()` — outputs all errors in `[line:col] Phase Error: message` format
- `hasErrors()` — used by the driver to abort/resume compilation

The error handler persists across compiler phases, allowing all errors from lexing, parsing, and semantic analysis to be reported at once.

### Phase 7: Code Generator (TAC)

**Files:** `include/CodeGenerator.h`, `src/CodeGenerator.cpp`

Generates Three Address Code (TAC) by walking the AST:

**Instruction Set:**

| Instruction | Format | Purpose |
|-------------|--------|---------|
| ASSIGN | `x = y` | Copy value |
| ADD/SUB/MUL/DIV | `t = a op b` | Arithmetic |
| EQ/NEQ/LT/GT/LEQ/GEQ | `t = a op b` | Comparison |
| AND/OR | `t = a op b` | Logical |
| UMINUS/NOT | `t = op a` | Unary |
| IF_FALSE | `if (!a) goto L` | Conditional jump |
| JUMP | `goto L` | Unconditional jump |
| LABEL | `L:` | Jump target |
| RET | `return [a]` | Function return |
| INPUT | `input x` | Read from stdin |
| OUTPUT | `output a` | Write to stdout |
| PARAM | `param x` | Function parameter |
| ARG | `arg x` | Function call argument |
| CALL | `t = call f` | Function invocation |

Temporary variables (`t0`, `t1`, ...) are generated for intermediate results. Labels (`L0`, `L1`, ...) are used for control flow targets.

### Driver / Pipeline (index.cpp)

**Files:** `index.cpp`

Orchestrates the compilation pipeline in 4 sequential phases:

1. **Phase 1 — Lexical Analysis:** Reads source, tokenizes, checks for illegal characters
2. **Phase 2 — Syntax Analysis:** Recursive descent parser builds AST from token stream
3. **Phase 3 — Semantic Analysis:** Type checking, variable resolution, using the Symbol Table
4. **Phase 4 — Code Generation:** Walks the AST and emits Three Address Code (TAC) instructions

Supporting components that run throughout: `ErrorHandler` (collects errors from all phases) and the `SymbolTable` (used by the semantic analyzer for scope management).

## 4. Test Results

### Valid Programs

| Test | Description | Result |
|------|-------------|--------|
| `hello.c` | Variable decl, assign, output, return | ✅ Compiles |
| `factorial.c` | While loop, function call, recursion | ✅ Compiles |
| `if_else.c` | If-else with comparison | ✅ Compiles |
| `for_loop.c` | For loop with increment | ✅ Compiles |
| `fibonacci.c` | Multiple blocks, while loop | ✅ Compiles |
| `bool_test.c` | Bool decl, if/while with bool, logical ops | ✅ Compiles |
| `all_constructs.c` | Mixed arithmetic, logical, if-else | ✅ Compiles |

### Invalid Programs

| Test | Error Type | Expected | Actual |
|------|-----------|----------|--------|
| `missing_semicolon.c` | Syntax | Missing `;` after declaration | ✅ Detected |
| `unmatched_paren.c` | Syntax | Unmatched `(` | ✅ Detected |
| `type_mismatch.c` | Semantic | `int = bool` and `bool = int` | ✅ Detected |
| `undeclared_var.c` | Semantic | Variable not declared | ✅ Detected |
| `redeclaration.c` | Semantic | Variable redeclared in same scope | ✅ Detected |
| `return_type_mismatch.c` | Semantic | Return type mismatch + assignment mismatch | ✅ Detected |

### Sample TAC Output (factorial.c)

```
factorial_entry:
  param n
  t0 = 1
  result = t0
L0:
  t1 = 0
  t2 = n GT t1
  if (!t2) goto L1
  t3 = result MUL n
  result = t3
  t4 = 1
  t5 = n SUB t4
  n = t5
  goto L0
L1:
  return result
  return
main_entry:
  input x
  arg x
  t6 = call factorial
  fact = t6
  output fact
  t7 = 0
  return t7
  return
```

## 5. Error Handling Approach

### Lexical Errors
- Illegal characters (e.g., `@`, `#`) produce `ILLEGAL` tokens
- Each illegal character is reported with its line and column

### Syntax Errors
- **Panic-mode recovery:** When an unexpected token is encountered, the parser skips tokens until a synchronization point (`;` or `}`) is reached
- **Specific error messages:** Each `expect()` call provides context (e.g., "Expected ';' after declaration")

### Semantic Errors
- **Type mismatches:** Reported in assignments, declarations, return statements, and operations
- **Undeclared variables:** Caught through symbol table lookup
- **Redeclaration:** Prevented by checking current scope before insertion
- **Operator type checking:** Logical operators require boolean operands, arithmetic requires numeric

### Error Output Format
```
[line:col] Phase Error: description
```

The compiler continues after errors to find as many issues as possible in a single run.

## 6. Comparison of Techniques: Hand-Crafted vs Tools

| Aspect | Hand-Crafted (This Project) | Tools (Lex/Yacc, ANTLR) |
|--------|---------------------------|------------------------|
| **Learning Value** | Deep understanding of all phases | Abstraction hides implementation details |
| **Code Size** | ~1500 lines total | Smaller specification files |
| **Flexibility** | Full control over error recovery, AST structure | Constrained by tool's output format |
| **Error Messages** | Custom, context-aware | Generic, less precise |
| **Development Speed** | Slower to implement | Faster initial development |
| **Grammar Changes** | Manual parser updates needed | Regenerate from grammar file |
| **Dependencies** | None (pure C++) | Requires tools and runtime libraries |
| **Performance** | Comparable | Comparable |

### Conclusion

For this educational project, the hand-crafted approach was chosen to gain a thorough understanding of compiler internals. Every phase — from tokenization to code generation — was implemented from scratch, providing insight into how real compilers work. The trade-off is more code to maintain, but with the benefit of complete control over error reporting, AST structure, and intermediate representation.

## 7. Project Structure

```
CC_LAB/
├── index.cpp                 # Driver / main entry point
├── Makefile                  # Build system
├── LANGUAGE.md               # Language definition & BNF grammar
├── include/                  # Header files
│   ├── Token.h               # (incorporated into Lexer.h)
│   ├── Lexer.h               # Token types, Token struct, Lexer class
│   ├── AST.h                 # AST node hierarchy
│   ├── Parser.h              # Recursive descent parser
│   ├── SymbolTable.h         # Symbol table with scopes
│   ├── SemanticAnalyzer.h    # Type checker
│   ├── CodeGenerator.h       # TAC code generator
│   └── ErrorHandler.h        # Centralized error reporting
├── src/                      # Implementations
│   ├── Lexer.cpp
│   ├── Parser.cpp
│   ├── SymbolTable.cpp
│   ├── SemanticAnalyzer.cpp
│   └── CodeGenerator.cpp
├── tests/                    # Test cases
│   ├── valid/                # Should compile cleanly
│   │   ├── hello.c
│   │   ├── factorial.c
│   │   ├── if_else.c
│   │   ├── for_loop.c
│   │   ├── fibonacci.c
│   │   ├── bool_test.c
│   │   └── all_constructs.c
│   └── invalid/              # Should produce errors
│       ├── missing_semicolon.c
│       ├── unmatched_paren.c
│       ├── type_mismatch.c
│       ├── undeclared_var.c
│       ├── redeclaration.c
│       └── return_type_mismatch.c
└── report.md                 # This report
```

## 8. Build & Run Instructions

```bash
# Build the compiler
make

# Compile a source file
./compiler tests/valid/hello.c

# Run all tests
make test

# Clean build artifacts
make clean
```

**Compiler:** g++ (C++11)
**Platform:** macOS / Linux
**Dependencies:** None (standard library only)
