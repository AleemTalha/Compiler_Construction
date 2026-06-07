// ============================================================
// CodeGenerator.cpp -- TAC Code Generation Implementation
// ============================================================
// Phase 7: Walks the AST and generates Three Address Code.
// Each TAC instruction holds: op, arg1, arg2, result, label.
// Temporaries (t0, t1...) are created for intermediate values.
// Labels (L0, L1...) are used for control flow targets.
// ============================================================

#include "../include/CodeGenerator.h"

CodeGenerator::CodeGenerator()
    : instCount(0), tempCount(0), labelCount(0) {}

string CodeGenerator::newTemp()
{
    return "t" + to_string(tempCount++);
}

string CodeGenerator::newLabel()
{
    return "L" + to_string(labelCount++);
}

void CodeGenerator::generate(Program* program)
{
    for (int i = 0; i < program->funcCount; i++)
        genFunctionDecl(program->functions[i]);
}

void CodeGenerator::genFunctionDecl(FunctionDecl* func)
{
    // Label marking function entry point
    if (instCount < MAX_INSTRUCTIONS) {
        instructions[instCount].op = "LABEL";
        instructions[instCount].label = func->name + "_entry";
        instCount++;
    }

    // Declare parameters
    for (int i = 0; i < func->paramCount; i++)
    {
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "PARAM";
            instructions[instCount].arg1 = func->params[i].name;
            instCount++;
        }
    }

    // Generate code for function body
    genStmt(func->body);

    // End of function
    if (instCount < MAX_INSTRUCTIONS) {
        instructions[instCount].op = "RET";
        instCount++;
    }
}

void CodeGenerator::genStmt(Stmt* stmt)
{
    if (!stmt) return;

    // === Block Statement ===
    BlockStmt* block = dynamic_cast<BlockStmt*>(stmt);
    if (block)
    {
        Stmt* cur = block->first;
        while (cur)
        {
            genStmt(cur);
            cur = cur->next;
        }
        return;
    }

    // === Declaration (with optional initializer) ===
    DeclStmt* decl = dynamic_cast<DeclStmt*>(stmt);
    if (decl)
    {
        if (decl->init)
        {
            string rhs = genExpr(decl->init);
            if (instCount < MAX_INSTRUCTIONS) {
                instructions[instCount].op = "ASSIGN";
                instructions[instCount].arg1 = rhs;
                instructions[instCount].result = decl->name;
                instCount++;
            }
        }
        return;
    }

    // === Assignment ===
    AssignStmt* assign = dynamic_cast<AssignStmt*>(stmt);
    if (assign)
    {
        string rhs = genExpr(assign->value);
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "ASSIGN";
            instructions[instCount].arg1 = rhs;
            instructions[instCount].result = assign->name;
            instCount++;
        }
        return;
    }

    // === If-Else Statement ===
    IfStmt* ifStmt = dynamic_cast<IfStmt*>(stmt);
    if (ifStmt)
    {
        string cond = genExpr(ifStmt->cond);
        string elseLabel = newLabel();
        string endLabel = newLabel();

        // Jump to else if condition is false
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "IF_FALSE";
            instructions[instCount].arg1 = cond;
            instructions[instCount].result = elseLabel;
            instCount++;
        }

        // Then branch
        genStmt(ifStmt->thenStmt);

        // Skip else after then
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "JUMP";
            instructions[instCount].result = endLabel;
            instCount++;
        }

        // Else label and branch
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "LABEL";
            instructions[instCount].label = elseLabel;
            instCount++;
        }

        if (ifStmt->elseStmt)
            genStmt(ifStmt->elseStmt);

        // End label
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "LABEL";
            instructions[instCount].label = endLabel;
            instCount++;
        }
        return;
    }

    // === While Loop ===
    WhileStmt* whileStmt = dynamic_cast<WhileStmt*>(stmt);
    if (whileStmt)
    {
        string startLabel = newLabel();
        string endLabel = newLabel();

        // Loop start label
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "LABEL";
            instructions[instCount].label = startLabel;
            instCount++;
        }

        // Check condition
        string cond = genExpr(whileStmt->cond);

        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "IF_FALSE";
            instructions[instCount].arg1 = cond;
            instructions[instCount].result = endLabel;
            instCount++;
        }

        // Loop body
        genStmt(whileStmt->body);

        // Jump back to condition check
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "JUMP";
            instructions[instCount].result = startLabel;
            instCount++;
        }

        // End label
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "LABEL";
            instructions[instCount].label = endLabel;
            instCount++;
        }
        return;
    }

    // === For Loop ===
    ForStmt* forStmt = dynamic_cast<ForStmt*>(stmt);
    if (forStmt)
    {
        string startLabel = newLabel();
        string endLabel = newLabel();

        // Initialization
        genStmt(forStmt->init);

        // Loop start label
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "LABEL";
            instructions[instCount].label = startLabel;
            instCount++;
        }

        // Condition check
        if (forStmt->cond)
        {
            string cond = genExpr(forStmt->cond);
            if (instCount < MAX_INSTRUCTIONS) {
                instructions[instCount].op = "IF_FALSE";
                instructions[instCount].arg1 = cond;
                instructions[instCount].result = endLabel;
                instCount++;
            }
        }

        // Loop body
        genStmt(forStmt->body);
        genStmt(forStmt->update);

        // Jump back to condition
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "JUMP";
            instructions[instCount].result = startLabel;
            instCount++;
        }

        // End label
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "LABEL";
            instructions[instCount].label = endLabel;
            instCount++;
        }
        return;
    }

    // === Return Statement ===
    ReturnStmt* returnStmt = dynamic_cast<ReturnStmt*>(stmt);
    if (returnStmt)
    {
        if (returnStmt->value)
        {
            string val = genExpr(returnStmt->value);
            if (instCount < MAX_INSTRUCTIONS) {
                instructions[instCount].op = "RET";
                instructions[instCount].arg1 = val;
                instCount++;
            }
        }
        else
        {
            if (instCount < MAX_INSTRUCTIONS) {
                instructions[instCount].op = "RET";
                instCount++;
            }
        }
        return;
    }

    // === Input Statement ===
    InputStmt* inputStmt = dynamic_cast<InputStmt*>(stmt);
    if (inputStmt)
    {
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "INPUT";
            instructions[instCount].result = inputStmt->name;
            instCount++;
        }
        return;
    }

    // === Output Statement ===
    OutputStmt* outputStmt = dynamic_cast<OutputStmt*>(stmt);
    if (outputStmt)
    {
        string expr = genExpr(outputStmt->expr);
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "OUTPUT";
            instructions[instCount].arg1 = expr;
            instCount++;
        }
        return;
    }
}

// Generate TAC for an expression. Returns the name of the
// temporary variable or identifier holding the result.
string CodeGenerator::genExpr(Expr* expr)
{
    if (!expr) return "0";

    // Integer literal
    if (dynamic_cast<IntLiteral*>(expr))
    {
        IntLiteral* lit = static_cast<IntLiteral*>(expr);
        string t = newTemp();
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "ASSIGN";
            instructions[instCount].arg1 = to_string(lit->value);
            instructions[instCount].result = t;
            instCount++;
        }
        return t;
    }

    // Float literal
    if (dynamic_cast<FloatLiteral*>(expr))
    {
        FloatLiteral* lit = static_cast<FloatLiteral*>(expr);
        string t = newTemp();
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "ASSIGN";
            instructions[instCount].arg1 = to_string(lit->value);
            instructions[instCount].result = t;
            instCount++;
        }
        return t;
    }

    // Boolean literal
    if (dynamic_cast<BoolLiteral*>(expr))
    {
        BoolLiteral* lit = static_cast<BoolLiteral*>(expr);
        string t = newTemp();
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "ASSIGN";
            instructions[instCount].arg1 = lit->value ? "true" : "false";
            instructions[instCount].result = t;
            instCount++;
        }
        return t;
    }

    // Variable reference
    if (dynamic_cast<Identifier*>(expr))
    {
        Identifier* id = static_cast<Identifier*>(expr);
        return id->name;    // Return the variable name directly
    }

    // Unary expression (-x or !flag)
    if (dynamic_cast<UnaryExpr*>(expr))
    {
        UnaryExpr* unary = static_cast<UnaryExpr*>(expr);
        string operand = genExpr(unary->operand);
        string t = newTemp();

        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = (unary->op == MINUS) ? "UMINUS" : "NOT";
            instructions[instCount].arg1 = operand;
            instructions[instCount].result = t;
            instCount++;
        }

        return t;
    }

    // Binary expression (a + b, x > y, etc.)
    if (dynamic_cast<BinaryExpr*>(expr))
    {
        BinaryExpr* binary = static_cast<BinaryExpr*>(expr);
        string left = genExpr(binary->left);
        string right = genExpr(binary->right);
        string t = newTemp();

        // Map token type to TAC operation name
        string opStr;
        switch (binary->op)
        {
            case PLUS:    opStr = "ADD";  break;
            case MINUS:   opStr = "SUB";  break;
            case STAR:    opStr = "MUL";  break;
            case SLASH:   opStr = "DIV";  break;
            case EQ:      opStr = "EQ";   break;
            case NEQ:     opStr = "NEQ";  break;
            case LT:      opStr = "LT";   break;
            case GT:      opStr = "GT";   break;
            case LEQ:     opStr = "LEQ";  break;
            case GEQ:     opStr = "GEQ";  break;
            case AND:     opStr = "AND";  break;
            case OR:      opStr = "OR";   break;
            default:      opStr = "NOP";  break;
        }

        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = opStr;
            instructions[instCount].arg1 = left;
            instructions[instCount].arg2 = right;
            instructions[instCount].result = t;
            instCount++;
        }

        return t;
    }

    // Function call
    if (dynamic_cast<FunCallExpr*>(expr))
    {
        FunCallExpr* funcCall = static_cast<FunCallExpr*>(expr);

        // Push arguments
        for (int i = 0; i < funcCall->argCount; i++)
        {
            string argVal = genExpr(funcCall->args[i]);
            if (instCount < MAX_INSTRUCTIONS) {
                instructions[instCount].op = "ARG";
                instructions[instCount].arg1 = argVal;
                instCount++;
            }
        }

        // Call the function
        string t = newTemp();
        if (instCount < MAX_INSTRUCTIONS) {
            instructions[instCount].op = "CALL";
            instructions[instCount].arg1 = funcCall->name;
            instructions[instCount].result = t;
            instCount++;
        }

        return t;
    }

    return "0";
}

// Print all generated TAC instructions
void CodeGenerator::printCode() const
{
    cout << "\n=== Three Address Code (TAC) ===\n" << endl;

    for (int i = 0; i < instCount; i++)
    {
        const TACInst& inst = instructions[i];

        // Labels are printed on their own line
        if (inst.op == "LABEL")
        {
            cout << inst.label << ":" << endl;
            continue;
        }

        cout << "  ";

        if (inst.op == "ASSIGN")
            cout << inst.result << " = " << inst.arg1;

        else if (inst.op == "UMINUS")
            cout << inst.result << " = -" << inst.arg1;

        else if (inst.op == "NOT")
            cout << inst.result << " = !" << inst.arg1;

        else if (inst.op == "INPUT")
            cout << "input " << inst.result;

        else if (inst.op == "OUTPUT")
            cout << "output " << inst.arg1;

        else if (inst.op == "RET")
        {
            if (!inst.arg1.empty())
                cout << "return " << inst.arg1;
            else
                cout << "return";
        }

        else if (inst.op == "IF_FALSE")
            cout << "if (!" << inst.arg1 << ") goto " << inst.result;

        else if (inst.op == "JUMP")
            cout << "goto " << inst.result;

        else if (inst.op == "PARAM")
            cout << "param " << inst.arg1;

        else if (inst.op == "ARG")
            cout << "arg " << inst.arg1;

        else if (inst.op == "CALL")
            cout << inst.result << " = call " << inst.arg1;

        else
            cout << inst.result << " = " << inst.arg1 << " "
                 << inst.op << " " << inst.arg2;

        cout << endl;
    }
}
