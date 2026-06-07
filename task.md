# Mini-Compiler for C-like Language — Project Guide

**Deadline:** 07-06-2026 (Today)
**Group Size:** 3 members

---

## Project Structure

```
CC_LAB/
├── index.cpp              # Driver / main entry point
├── include/               # Header files
│   ├── Token.h            # Token types & structure
│   ├── Lexer.h            # Lexical analyzer
│   ├── AST.h              # Abstract Syntax Tree nodes
│   ├── Parser.h           # Recursive descent parser
│   ├── SymbolTable.h      # Symbol table manager
│   ├── SemanticAnalyzer.h # Type checking & scope
│   ├── CodeGenerator.h    # Intermediate code gen (TAC)
│   └── ErrorHandler.h     # Error reporting & recovery
├── src/                   # Implementation files
│   ├── main.cpp
│   ├── Lexer.cpp
│   ├── Parser.cpp
│   ├── SymbolTable.cpp
│   ├── SemanticAnalyzer.cpp
│   └── CodeGenerator.cpp
├── tests/                 # Test cases
│   ├── valid/             # Should compile cleanly
│   └── invalid/           # Should produce errors
└── report.md              # Final report
```

---

## Task Breakdown (Do in order)

### Phase 0: Setup & Language Definition
- [ ] **0.1** Define the exact subset of C-like language (BNF grammar)
- [ ] **0.2** List all token types, keywords, operators
- [ ] **0.3** Set up build system (g++ / CMake)

### Phase 1: Lexical Analyzer (Scanner)
- [ ] **1.1** Define `Token` struct (type, lexeme, line, col)
- [ ] **1.2** Implement `Lexer` class — reads char-by-char, produces token stream
- [ ] **1.3** Handle: identifiers, keywords, int/float/bool literals, operators, delimiters
- [ ] **1.4** Handle: whitespace skipping, single-line & multi-line comments
- [ ] **1.5** Report lexical errors (illegal characters with line number)

### Phase 2: Abstract Syntax Tree (AST)
- [ ] **2.1** Define AST node base class and derived node types
- [ ] **2.2** Nodes for: expressions (binary, unary, literals, identifiers)
- [ ] **2.3** Nodes for: statements (if, while, for, assign, decl, input/output, block)
- [ ] **2.4** Nodes for: functions (declaration, call, return)

### Phase 3: Syntax Analyzer (Parser)
- [ ] **3.1** Write grammar rules in BNF for the language
- [ ] **3.2** Implement recursive descent parser functions (one per rule)
- [ ] **3.3** Build AST during parsing
- [ ] **3.4** Syntax error detection (missing semicolons, unmatched parens, etc.)
- [ ] **3.5** Error recovery (panic mode — skip to next `;` or `}`)

### Phase 4: Symbol Table
- [ ] **4.1** Implement `Symbol` struct (name, type, scope, line declared)
- [ ] **4.2** Implement `SymbolTable` with nested scopes (push/pop)
- [ ] **4.3** Operations: insert, lookup, remove scope

### Phase 5: Semantic Analyzer
- [ ] **5.1** Walk the AST and perform type checking
- [ ] **5.2** Check: variable declared before use
- [ ] **5.3** Check: type compatibility in assignments & expressions
- [ ] **5.4** Check: function call arg count & types
- [ ] **5.5** Check: return type matching
- [ ] **5.6** Report semantic errors with context

### Phase 6: Error Handler
- [ ] **6.1** Centralized `ErrorHandler` class
- [ ] **6.2** Collect errors from all phases (lexical, syntax, semantic)
- [ ] **6.3** Print errors with: `[line:col] Error: description`
- [ ] **6.4** Error recovery: continue after errors to find more

### Phase 7: Intermediate Code Generator
- [ ] **7.1** Design TAC instruction set (label, op, arg1, arg2, result)
- [ ] **7.2** Walk AST and emit TAC instructions
- [ ] **7.3** Handle: expressions, conditionals (if/while/for), I/O
- [ ] **7.4** Output TAC to console or file

### Phase 8: Driver (main.cpp)
- [ ] **8.1** Read source file
- [ ] **8.2** Run Lexer → Parser → SemanticAnalyzer → CodeGenerator
- [ ] **8.3** Print token stream (optional debug)
- [ ] **8.4** Print AST (optional debug)
- [ ] **8.5** Print errors or "Compilation Successful"

### Phase 9: Test Cases
- [ ] **9.1** Valid programs (hello world, factorial, all constructs)
- [ ] **9.2** Invalid programs (missing semicolon, type mismatch, undeclared var)

### Phase 10: Report
- [ ] **10.1** Design choices & architecture overview
- [ ] **10.2** Grammar rules (BNF)
- [ ] **10.3** Implementation details per phase
- [ ] **10.4** Test results & screenshots
- [ ] **10.5** Error handling approach
- [ ] **10.6** Comparison of techniques (hand-crafted vs tools)

---

## First Step: DEFINE THE LANGUAGE (0.1)

Before coding anything, we must define **exactly** what our language looks like.

Example valid program:
```
int factorial(int n) {
    int result = 1;
    while (n > 0) {
        result = result * n;
        n = n - 1;
    }
    return result;
}

int main() {
    int x;
    input(x);
    int fact = factorial(x);
    output(fact);
    return 0;
}
```

We need BNF grammar for:
- Program structure
- Functions (with params & return types)
- Statements (declaration, assignment, if, while, for, input, output, return, block)
- Expressions (arithmetic, relational, logical)

---

## Division of Work (for 3 members)

| Member | Primary Tasks |
|--------|---------------|
| **A** | Lexer + Token + ErrorHandler |
| **B** | Parser + AST |
| **C** | SymbolTable + SemanticAnalyzer + CodeGenerator |
| **All** | Tests + Report + main.cpp integration |

---

**Start with Task 0.1: Write the BNF grammar. Let me know when you're ready.**
