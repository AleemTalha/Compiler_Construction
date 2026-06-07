// ============================================================
// Parser.cpp -- Recursive Descent Parser Implementation
// ============================================================
// Phase 3: Parses the token stream into an AST.
// One function per grammar rule (recursive descent).
// Implements panic-mode error recovery on syntax errors.
// ============================================================

#include "../include/Parser.h"

Parser::Parser() : tokenCount(0), pos(0), errors(nullptr) {}

void Parser::setTokens(const Token* tkns, int count, ErrorHandler* err)
{
    for (int i = 0; i < count && i < MAX_TOKENS; i++)
        tokens[i] = tkns[i];
    tokenCount = count;
    pos = 0;
    errors = err;
}

Token Parser::peek() const
{
    if (pos >= tokenCount)
        return {END_OF_FILE, "", 1, 1};
    return tokens[pos];
}

Token Parser::advance()
{
    if (pos < tokenCount) pos++;
    return previous();
}

Token Parser::previous() const
{
    if (pos == 0 || pos - 1 >= tokenCount)
        return {END_OF_FILE, "", 1, 1};
    return tokens[pos - 1];
}

bool Parser::isAtEnd() const
{
    return pos >= tokenCount || tokens[pos].type == END_OF_FILE;
}

Token Parser::expect(TokenType type, const string& errorMsg)
{
    if (peek().type == type)
        return advance();

    errors->addError(peek().line, peek().col, "Syntax", errorMsg);
    return {ILLEGAL, "", peek().line, peek().col};
}

bool Parser::match(TokenType type)
{
    if (peek().type == type)
    {
        advance();
        return true;
    }
    return false;
}

void Parser::sync()
{
    while (!isAtEnd())
    {
        TokenType t = peek().type;
        if (t == SEMICOLON || t == RBRACE || t == END_OF_FILE)
            return;
        advance();
    }
}

TokenType Parser::parseType()
{
    if (peek().type == INT || peek().type == FLOAT ||
        peek().type == BOOL || peek().type == VOID)
    {
        return advance().type;
    }
    errors->addError(peek().line, peek().col, "Syntax",
        "Expected type (int, float, bool, void)");
    return INT;
}

Program* Parser::parse()
{
    Program* program = new Program();

    while (!isAtEnd() && peek().type != END_OF_FILE)
    {
        if (peek().type == INT || peek().type == FLOAT ||
            peek().type == BOOL || peek().type == VOID)
        {
            FunctionDecl* func = parseFunctionDecl();
            if (func)
                program->addFunction(func);
            else
                sync();
        }
        else
        {
            errors->addError(peek().line, peek().col, "Syntax",
                "Expected function declaration (type) at top level");
            sync();
        }
    }

    if (program->funcCount == 0)
    {
        errors->addError(1, 1, "Syntax",
            "Program must contain at least one function declaration");
    }

    return program;
}

FunctionDecl* Parser::parseFunctionDecl()
{
    TokenType retType = parseType();
    Token nameToken = expect(ID, "Expected function name");
    string funcName = nameToken.lexeme;

    expect(LPAREN, "Expected '(' after function name");

    FunctionDecl* func = new FunctionDecl(retType, funcName, nullptr);

    parseParamList(func);

    expect(RPAREN, "Expected ')' after parameter list");

    Stmt* bodyStmt = parseBlockStmt();
    BlockStmt* body = dynamic_cast<BlockStmt*>(bodyStmt);

    if (body == nullptr)
    {
        errors->addError(peek().line, peek().col, "Syntax",
            "Expected function body for '" + funcName + "'");
        delete func;
        return nullptr;
    }

    func->body = body;
    return func;
}

void Parser::parseParamList(FunctionDecl* func)
{
    if (peek().type == RPAREN)
        return;

    {
        TokenType type = parseType();
        Token name = expect(ID, "Expected parameter name");
        func->addParam(Param(type, name.lexeme));
    }

    while (match(COMMA))
    {
        TokenType type = parseType();
        Token name = expect(ID, "Expected parameter name");
        func->addParam(Param(type, name.lexeme));
    }
}

Stmt* Parser::parseStmt()
{
    switch (peek().type)
    {
        case INT:
        case FLOAT:
        case BOOL:
            return parseDeclStmt();

        case IF:
            return parseIfStmt();

        case WHILE:
            return parseWhileStmt();

        case FOR:
            return parseForStmt();

        case RETURN:
            return parseReturnStmt();

        case INPUT:
            return parseInputStmt();

        case OUTPUT:
            return parseOutputStmt();

        case LBRACE:
            return parseBlockStmt();

        case ID:
        {
            string name = peek().lexeme;
            advance();
            if (peek().type == ASSIGN)
                return parseAssignStmt(name);

            errors->addError(peek().line, peek().col, "Syntax",
                "Expected '=' after identifier in statement");
            sync();
            return nullptr;
        }

        case SEMICOLON:
            advance();
            return nullptr;

        default:
            errors->addError(peek().line, peek().col, "Syntax",
                "Unexpected token '" + peek().lexeme + "'");
            sync();
            return nullptr;
    }
}

Stmt* Parser::parseDeclStmt()
{
    TokenType type = parseType();
    Token name = expect(ID, "Expected variable name");
    int line = name.line;

    Expr* init = nullptr;

    if (match(ASSIGN))
        init = parseExpr();

    expect(SEMICOLON, "Expected ';' after declaration");
    if (previous().type != SEMICOLON)
    {
        delete init;
        return nullptr;
    }

    return new DeclStmt(type, name.lexeme, line, init);
}

Stmt* Parser::parseAssignStmt(const string& name)
{
    advance();
    Expr* value = parseExpr();
    expect(SEMICOLON, "Expected ';' after assignment");
    return new AssignStmt(name, value);
}

Stmt* Parser::parseIfStmt()
{
    advance();
    expect(LPAREN, "Expected '(' after 'if'");
    Expr* cond = parseExpr();
    expect(RPAREN, "Expected ')' after condition");

    Stmt* thenStmt = parseStmt();
    Stmt* elseStmt = nullptr;

    if (match(ELSE))
        elseStmt = parseStmt();

    return new IfStmt(cond, thenStmt, elseStmt);
}

Stmt* Parser::parseWhileStmt()
{
    advance();
    expect(LPAREN, "Expected '(' after 'while'");
    Expr* cond = parseExpr();
    expect(RPAREN, "Expected ')' after condition");

    Stmt* body = parseStmt();
    return new WhileStmt(cond, body);
}

Stmt* Parser::parseForStmt()
{
    advance();
    expect(LPAREN, "Expected '(' after 'for'");

    Stmt* init = nullptr;
    if (peek().type == INT || peek().type == FLOAT || peek().type == BOOL)
        init = parseDeclStmt();
    else if (peek().type == ID)
    {
        string name = peek().lexeme;
        advance();
        if (peek().type == ASSIGN)
            init = parseAssignStmt(name);
        else
        {
            pos--;
            expect(SEMICOLON, "Expected ';' in for loop");
        }
    }
    else
        expect(SEMICOLON, "Expected ';' in for loop");

    Expr* cond = nullptr;
    if (peek().type != SEMICOLON)
        cond = parseExpr();
    expect(SEMICOLON, "Expected ';' after condition in for loop");

    Stmt* update = nullptr;
    if (peek().type != RPAREN)
    {
        if (peek().type == ID)
        {
            string name = peek().lexeme;
            advance();
            if (peek().type == ASSIGN)
            {
                advance();
                Expr* val = parseExpr();
                update = new AssignStmt(name, val);
            }
        }
    }
    expect(RPAREN, "Expected ')' after for loop clause");

    Stmt* body = parseStmt();
    return new ForStmt(init, cond, update, body);
}

Stmt* Parser::parseReturnStmt()
{
    advance();

    Expr* value = nullptr;
    if (peek().type != SEMICOLON)
        value = parseExpr();

    expect(SEMICOLON, "Expected ';' after return");
    return new ReturnStmt(value);
}

Stmt* Parser::parseInputStmt()
{
    advance();
    expect(LPAREN, "Expected '(' after 'input'");
    Token name = expect(ID, "Expected variable name inside input()");
    expect(RPAREN, "Expected ')' after input argument");
    expect(SEMICOLON, "Expected ';' after input statement");

    return new InputStmt(name.lexeme);
}

Stmt* Parser::parseOutputStmt()
{
    advance();

    if (previous().type != OUTPUT)
        return nullptr;

    expect(LPAREN, "Expected '(' after 'output'");
    Expr* expr = parseExpr();
    expect(RPAREN, "Expected ')' after output argument");
    expect(SEMICOLON, "Expected ';' after output statement");

    return new OutputStmt(expr);
}

Stmt* Parser::parseBlockStmt()
{
    expect(LBRACE, "Expected '{' to start block");

    Stmt* first = nullptr;
    Stmt* last = nullptr;

    while (!isAtEnd() && peek().type != RBRACE)
    {
        Stmt* stmt = parseStmt();
        if (stmt)
        {
            if (first == nullptr)
            {
                first = stmt;
                last = stmt;
            }
            else
            {
                last->next = stmt;
                last = stmt;
            }
        }
    }

    expect(RBRACE, "Expected '}' to end block");
    return new BlockStmt(first);
}

Expr* Parser::parseExpr()
{
    return parseOrExpr();
}

Expr* Parser::parseOrExpr()
{
    Expr* left = parseAndExpr();

    while (match(OR))
    {
        TokenType op = previous().type;
        Expr* right = parseAndExpr();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}

Expr* Parser::parseAndExpr()
{
    Expr* left = parseEqExpr();

    while (match(AND))
    {
        TokenType op = previous().type;
        Expr* right = parseEqExpr();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}

Expr* Parser::parseEqExpr()
{
    Expr* left = parseRelExpr();

    if (match(EQ) || match(NEQ))
    {
        TokenType op = previous().type;
        Expr* right = parseRelExpr();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}

Expr* Parser::parseRelExpr()
{
    Expr* left = parseAddExpr();

    if (match(LT) || match(GT) || match(LEQ) || match(GEQ))
    {
        TokenType op = previous().type;
        Expr* right = parseAddExpr();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}

Expr* Parser::parseAddExpr()
{
    Expr* left = parseMulExpr();

    while (match(PLUS) || match(MINUS))
    {
        TokenType op = previous().type;
        Expr* right = parseMulExpr();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}

Expr* Parser::parseMulExpr()
{
    Expr* left = parseUnaryExpr();

    while (match(STAR) || match(SLASH))
    {
        TokenType op = previous().type;
        Expr* right = parseUnaryExpr();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}

Expr* Parser::parseUnaryExpr()
{
    if (match(NOT) || match(MINUS))
    {
        TokenType op = previous().type;
        Expr* operand = parseUnaryExpr();
        return new UnaryExpr(op, operand);
    }

    return parsePrimaryExpr();
}

Expr* Parser::parsePrimaryExpr()
{
    Token t = peek();

    if (match(INT_VAL))
    {
        int val = stoi(t.lexeme);
        return new IntLiteral(val);
    }

    if (match(FLOAT_VAL))
    {
        float val = stof(t.lexeme);
        return new FloatLiteral(val);
    }

    if (match(TRUE))
        return new BoolLiteral(true);

    if (match(FALSE))
        return new BoolLiteral(false);

    if (match(LPAREN))
    {
        Expr* expr = parseExpr();
        expect(RPAREN, "Expected ')' after expression");
        return expr;
    }

    if (match(ID))
    {
        string name = t.lexeme;
        if (peek().type == LPAREN)
            return parseFunCallExpr(name);
        return new Identifier(name);
    }

    errors->addError(peek().line, peek().col, "Syntax",
        "Expected expression, got '" + peek().lexeme + "'");
    return new IntLiteral(0);
}

Expr* Parser::parseFunCallExpr(const string& name)
{
    advance();
    FunCallExpr* call = new FunCallExpr(name);

    if (peek().type != RPAREN)
    {
        call->addArg(parseExpr());
        while (match(COMMA))
            call->addArg(parseExpr());
    }

    expect(RPAREN, "Expected ')' after function arguments");
    return call;
}
