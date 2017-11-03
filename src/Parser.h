#pragma once

#include "AST.h"
#include "PoolAllocator.h"
#include "Lexer.h"
#include "Array.h"

struct Parser {
    Lexer *lex;
    char errorString[512];
    Scope *current_scope;
    bool success;
    void Error(const char *msg, ...);

    bool MustMatchToken(TOKEN_TYPE type, char *msg = nullptr);
    bool AddDeclarationToScope(VariableDeclarationAST *decl);

    TypeAST *parseDirectType();
    TypeAST *parseType();
    ArgumentDeclarationAST *parseArgumentDeclaration();
    FunctionTypeAST *parseFunctionDeclaration();
    ReturnStatementAST *parseReturnStatement();
    StatementAST *parseStatement();
    StatementBlockAST *parseStatementBlock();
    FunctionDefinitionAST *parseFunctionDefinition();
    FunctionCallAST *parseFunctionCall();
    ExpressionAST * parseLiteral();
    ExpressionAST * parseUnaryExpression();
    ExpressionAST * parseBinOpExpressionRecursive(u32 oldprec, ExpressionAST *lhs);
    ExpressionAST * parseBinOpExpression();
    DefinitionAST * parseDefinition();
    VariableDeclarationAST * parseDeclaration();
    ExpressionAST * parseAssignmentExpression();
    ExpressionAST * parseExpression();

    FileAST * Parse(const char *filename, PoolAllocator *pool);
};

void traverseAST(FileAST *root);
