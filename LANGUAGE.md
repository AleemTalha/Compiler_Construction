# Language Definition — C-Like Subset

## 1. Token Types

```
Keywords:
  int, float, bool, void,
  if, else, while, for, return,
  input, output, true, false

Operators:
  +, -, *, /, =,
  ==, !=, <, >, <=, >=,
  &&, ||, !

Delimiters:
  ( , ) , { , } , ; , ,

Literals:
  INTEGER      → [0-9]+
  FLOAT        → [0-9]+.[0-9]+
  IDENTIFIER   → [a-zA-Z_][a-zA-Z0-9_]*

Other:
  EOF          → end of file
  ILLEGAL      → unrecognized character
```

## 2. BNF Grammar

```
Program       → { FunctionDecl }

FunctionDecl  → Type IDENTIFIER ( ParamList ) BlockStmt

ParamList     → ε | Param { , Param }
Param         → Type IDENTIFIER

Type          → int | float | bool | void

BlockStmt     → { { Stmt } }

Stmt          → DeclStmt
              | AssignStmt
              | IfStmt
              | WhileStmt
              | ForStmt
              | ReturnStmt
              | InputStmt
              | OutputStmt
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
PrimaryExpr   → IDENTIFIER
              | INTEGER
              | FLOAT
              | true
              | false
              | ( Expr )
              | FunCallExpr

FunCallExpr   → IDENTIFIER ( ArgList )
ArgList       → ε | Expr { , Expr }
```

## 3. Supported Features

| Feature | Example |
|---------|---------|
| Variable declaration | `int x;` `float y = 3.14;` `bool flag = true;` |
| Assignment | `x = 5 + 3;` |
| Arithmetic | `+`, `-`, `*`, `/` |
| Relational | `==`, `!=`, `<`, `>`, `<=`, `>=` |
| Logical | `&&`, `||`, `!` |
| If-else | `if (x > 0) { ... } else { ... }` |
| While | `while (i < 10) { ... }` |
| For | `for (i = 0; i < 10; i = i + 1) { ... }` |
| Functions | `int add(int a, int b) { return a + b; }` |
| Input | `input(x);` |
| Output | `output(x);` |
| Comments | `// single line` or `/* multi line */` |

## 4. Type Rules

```
int     → 32-bit signed integer
float   → 32-bit IEEE 754
bool    → true / false

Type coercion (semantic analysis):
  int + int     → int
  float + float → float
  int + float   → float (implicit)
  bool + int    → ERROR
  =             → RHS type must match LHS type or be coercible
```

## 5. Scoping Rules

- Block-scoped (each `{ }` opens a new scope)
- Variables must be declared before use
- No redeclaration in the same scope
- Inner scope can shadow outer scope
