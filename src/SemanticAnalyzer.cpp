// ============================================================
// SemanticAnalyzer.cpp -- Semantic Analysis Implementation
// ============================================================
// Phase 5: Walks the AST using the SymbolTable to verify:
//   - All variables are declared before use
//   - Types are compatible in assignments and expressions
//   - Return types match function declarations
//   - Operators are used with correct operand types
// ============================================================

#include "../include/SemanticAnalyzer.h"

SemanticAnalyzer::SemanticAnalyzer(ErrorHandler* err)
    : errors(err), currentReturnType(INT) {}

void SemanticAnalyzer::analyze(Program* program)
{
    symTable.pushScope();   // Global scope

    for (int i = 0; i < program->funcCount; i++)
        visitFunctionDecl(program->functions[i]);

    symTable.popScope();
}

bool SemanticAnalyzer::isNumeric(TokenType t) const
{
    return t == INT || t == FLOAT;
}

// Check if source type can be assigned to destination type
// Allows implicit int-to-float conversion
bool SemanticAnalyzer::canAssign(TokenType dest, TokenType src) const
{
    if (dest == src) return true;
    if (isNumeric(dest) && isNumeric(src)) return true;
    return false;
}

void SemanticAnalyzer::visitFunctionDecl(FunctionDecl* func)
{
    symTable.pushScope();                            // Function scope
    currentReturnType = func->returnType;            // Track expected return type

    // Insert parameters into symbol table
    for (int i = 0; i < func->paramCount; i++)
    {
        Symbol sym;
        sym.name = func->params[i].name;
        sym.type = func->params[i].type;
        sym.line = 1;
        sym.scopeLevel = symTable.getCurrentLevel();
        symTable.insert(sym, errors);
    }

    visitStmt(func->body);                           // Check function body
    symTable.popScope();
}

void SemanticAnalyzer::visitStmt(Stmt* stmt)
{
    if (!stmt) return;

    // === Declaration Statement ===
    DeclStmt* decl = dynamic_cast<DeclStmt*>(stmt);
    if (decl)
    {
        Symbol sym;
        sym.name = decl->name;
        sym.type = decl->type;
        sym.line = decl->line;
        sym.scopeLevel = symTable.getCurrentLevel();

        // Check redeclaration
        if (symTable.lookupCurrentScope(decl->name))
        {
            errors->addError(decl->line, 1, "Semantic",
                "Variable '" + decl->name + "' already declared in this scope");
        }
        else
        {
            symTable.insert(sym, errors);
        }

        // Check initializer type compatibility
        if (decl->init)
        {
            TokenType initType = visitExpr(decl->init);
            if (!canAssign(decl->type, initType))
            {
                errors->addError(decl->line, 1, "Semantic",
                    "Type mismatch in declaration of '" + decl->name + "'");
            }
        }

        return;
    }

    // === Block Statement (enter new scope) ===
    BlockStmt* block = dynamic_cast<BlockStmt*>(stmt);
    if (block)
    {
        symTable.pushScope();

        Stmt* cur = block->first;
        while (cur)
        {
            visitStmt(cur);
            cur = cur->next;
        }

        symTable.popScope();
        return;
    }

    // === Assignment Statement ===
    AssignStmt* assign = dynamic_cast<AssignStmt*>(stmt);
    if (assign)
    {
        Symbol* sym = symTable.lookup(assign->name);

        if (!sym)
        {
            errors->addError(1, 1, "Semantic",
                "Variable '" + assign->name + "' not declared");
            return;
        }

        TokenType rhsType = visitExpr(assign->value);

        if (!canAssign(sym->type, rhsType))
        {
            errors->addError(1, 1, "Semantic",
                "Type mismatch in assignment to '" + assign->name + "'");
        }

        return;
    }

    // === If Statement ===
    IfStmt* ifStmt = dynamic_cast<IfStmt*>(stmt);
    if (ifStmt)
    {
        TokenType condType = visitExpr(ifStmt->cond);

        if (condType != BOOL && condType != INT)
        {
            errors->addError(1, 1, "Semantic",
                "If condition must be boolean or integer");
        }

        visitStmt(ifStmt->thenStmt);
        visitStmt(ifStmt->elseStmt);
        return;
    }

    // === While Loop ===
    WhileStmt* whileStmt = dynamic_cast<WhileStmt*>(stmt);
    if (whileStmt)
    {
        TokenType condType = visitExpr(whileStmt->cond);

        if (condType != BOOL && condType != INT)
        {
            errors->addError(1, 1, "Semantic",
                "While condition must be boolean or integer");
        }

        visitStmt(whileStmt->body);
        return;
    }

    // === For Loop ===
    ForStmt* forStmt = dynamic_cast<ForStmt*>(stmt);
    if (forStmt)
    {
        visitStmt(forStmt->init);

        if (forStmt->cond)
        {
            TokenType condType = visitExpr(forStmt->cond);

            if (condType != BOOL && condType != INT)
            {
                errors->addError(1, 1, "Semantic",
                    "For condition must be boolean or integer");
            }
        }

        visitStmt(forStmt->update);
        visitStmt(forStmt->body);
        return;
    }

    // === Return Statement ===
    ReturnStmt* returnStmt = dynamic_cast<ReturnStmt*>(stmt);
    if (returnStmt)
    {
        if (returnStmt->value)
        {
            TokenType retExprType = visitExpr(returnStmt->value);

            if (!canAssign(currentReturnType, retExprType))
            {
                errors->addError(1, 1, "Semantic",
                    "Return type mismatch");
            }
        }
        else if (currentReturnType != VOID)
        {
            errors->addError(1, 1, "Semantic",
                "Non-void function must return a value");
        }

        return;
    }

    // === Input Statement ===
    InputStmt* inputStmt = dynamic_cast<InputStmt*>(stmt);
    if (inputStmt)
    {
        Symbol* sym = symTable.lookup(inputStmt->name);

        if (!sym)
        {
            errors->addError(1, 1, "Semantic",
                "Variable '" + inputStmt->name + "' not declared");
        }

        return;
    }

    // === Output Statement ===
    OutputStmt* outputStmt = dynamic_cast<OutputStmt*>(stmt);
    if (outputStmt)
    {
        visitExpr(outputStmt->expr);
        return;
    }
}

// Evaluate an expression and return its type.
// Also performs type checking for operator operands.
TokenType SemanticAnalyzer::visitExpr(Expr* expr)
{
    if (!expr) return VOID;

    // Literals return their obvious types
    if (dynamic_cast<IntLiteral*>(expr)) return INT;
    if (dynamic_cast<FloatLiteral*>(expr)) return FLOAT;
    if (dynamic_cast<BoolLiteral*>(expr)) return BOOL;

    // Identifier: look up its type in symbol table
    Identifier* id = dynamic_cast<Identifier*>(expr);
    if (id)
    {
        Symbol* sym = symTable.lookup(id->name);

        if (!sym)
        {
            errors->addError(1, 1, "Semantic",
                "Variable '" + id->name + "' not declared");
            return VOID;
        }

        return sym->type;
    }

    // Unary expression: check operand type for ! and -
    UnaryExpr* unary = dynamic_cast<UnaryExpr*>(expr);
    if (unary)
    {
        TokenType operandType = visitExpr(unary->operand);

        if (unary->op == NOT && operandType != BOOL)
        {
            errors->addError(1, 1, "Semantic",
                "Logical NOT requires boolean operand");
            return BOOL;
        }

        if (unary->op == MINUS && !isNumeric(operandType))
        {
            errors->addError(1, 1, "Semantic",
                "Arithmetic negation requires numeric operand");
            return INT;
        }

        return operandType;
    }

    // Binary expression: check both operands
    BinaryExpr* binary = dynamic_cast<BinaryExpr*>(expr);
    if (binary)
    {
        TokenType leftType = visitExpr(binary->left);
        TokenType rightType = visitExpr(binary->right);
        TokenType op = binary->op;

        // Logical operators (&&, ||) require boolean operands
        if (op == AND || op == OR)
        {
            if (leftType != BOOL || rightType != BOOL)
            {
                errors->addError(1, 1, "Semantic",
                    "Logical operators require boolean operands");
            }
            return BOOL;
        }

        // Equality operators (==, !=) require compatible types
        if (op == EQ || op == NEQ)
        {
            if (!canAssign(leftType, rightType) && !canAssign(rightType, leftType))
            {
                errors->addError(1, 1, "Semantic",
                    "Type mismatch in equality comparison");
            }
            return BOOL;
        }

        // Relational operators (<, >, <=, >=) require numeric operands
        if (op == LT || op == GT || op == LEQ || op == GEQ)
        {
            if (!isNumeric(leftType) || !isNumeric(rightType))
            {
                errors->addError(1, 1, "Semantic",
                    "Relational operators require numeric operands");
            }
            return BOOL;
        }

        // Arithmetic operators (+, -, *, /) require numeric operands
        if (op == PLUS || op == MINUS || op == STAR || op == SLASH)
        {
            if (!isNumeric(leftType) || !isNumeric(rightType))
            {
                errors->addError(1, 1, "Semantic",
                    "Arithmetic operators require numeric operands");
            }

            if (leftType == FLOAT || rightType == FLOAT)
                return FLOAT;
            return INT;
        }

        return leftType;
    }

    // Function call: check all arguments
    FunCallExpr* funcCall = dynamic_cast<FunCallExpr*>(expr);
    if (funcCall)
    {
        for (int i = 0; i < funcCall->argCount; i++)
            visitExpr(funcCall->args[i]);
        return INT;     // Simplified: assume functions return int
    }

    return VOID;
}
