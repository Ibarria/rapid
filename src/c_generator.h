#pragma once
#include "AST.h"
#include <stdio.h>
#include "FileObject.h"

class c_generator
{
    FILE *output_file;
    u32 ident;
    TextType last_filename;
    u32 last_linenum;

    Array<VariableDeclarationAST *> dangling_functions;
    bool insert_dangling_funcs;
    void generate_preamble();
    void do_ident();
    void generate_line_info(BaseAST *ast);
    void generate_dangling_functions();
    void ensure_deps_are_generated(StructTypeAST *stype);
    void generate_function_prototype(VariableDeclarationAST *decl, bool second_pass = false);
    void generate_struct_prototype(VariableDeclarationAST *decl);
    void generate_array_type_prototype(ArrayTypeAST *at = nullptr);
    void generate_variable_declaration(VariableDeclarationAST *decl);
    void generate_argument_declaration(VariableDeclarationAST *arg);
    void generate_statement_block(StatementBlockAST *block);
    void generate_statement(StatementAST *stmt);
    void generate_if_statement(IfStatementAST *ifst);
    void generate_return_statement(ReturnStatementAST *ret);
    void generate_assignment(AssignmentAST *assign);
    void generate_expression(ExpressionAST *expr);
    void generate_function_call(FunctionCallAST *call);
    void generate_type(BaseAST *ast);
public:
    void generate_c_file(FileObject &filename, FileAST *root);
};

