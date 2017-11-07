#include "bytecode_generator.h"
#include <string.h>

const u64 stack_size = 10 * 1024;

union __xxx_to_u64 {
    f64 f;
    s64 s;
    u64 u;
    void *p;
};

static u64 straight_convert(f64 f) {
    __xxx_to_u64 x;
    x.f = f;
    return x.u;
}

static u64 straight_convert(s64 s) {
    __xxx_to_u64 x;
    x.s = s;
    return x.u;
}

static u64 straight_convert(u64 u) {
    return u;
}

static u64 straight_convert(void *p) {
    __xxx_to_u64 x;
    x.p = p;
    return x.u;
}

static inline u8 truncate_op_size(s64 bits)
{
    s64 bytes = bits /= 8;
    if (bytes < 8) return (u8)bytes;
    return 8;
}

static u64 getScopeVariablesSize(Scope *scope)
{
    u64 total_size = 0;
    
    // this only works for the top scope for now due to computing the total_size
    // might work for stack... who knows
    assert(scope->parent == nullptr);

    for (auto decl : scope->decls) {
        assert(decl->specified_type);
        assert(decl->specified_type->size_in_bits > 0);
        assert((decl->specified_type->size_in_bits % 8) == 0);
        
        // the offset for global variables is the accumulated size in bytes
        decl->bc_mem_offset = total_size;

        total_size += decl->specified_type->size_in_bits / 8;

    }
    return total_size;
}

static inline u64 roundToPage(u64 size, u64 page_size)
{
    size = (page_size - 1)&size ? ((size + page_size) & ~(page_size - 1)) : size;
    return size;
}

BCI * bytecode_generator::create_instruction(BytecodeInstructionOpcode opcode, s16 src_reg, s16 dst_reg, u64 big_const)
{
    BCI *bci = new (pool) BCI;
    bci->opcode = opcode;
    bci->src_reg = src_reg;
    bci->dst_reg = dst_reg;
    bci->big_const = big_const;

    return bci;
}

void bytecode_generator::issue_instruction(BCI * bci)
{
    bci->inst_index = program->instructions.push_back(bci);
}

bytecode_program * bytecode_generator::compileToBytecode(FileAST * root)
{
    bytecode_program *bp = new (pool) bytecode_program;
    this->program = bp;

    // First step, have space in the bss for the global variables (non functions)
    u64 bss_size = getScopeVariablesSize(&root->scope);
    // ensure we get page aligned memory chunks
    bss_size = roundToPage(bss_size, 4 * 1024);

    bp->bss.initMem((u8 *)pool->alloc(bss_size + stack_size),
        bss_size, stack_size);
    
    // set the correct value in the bss area for vars (bytecode instruction)
    initializeVariablesInScope(&root->scope);

    // Next, write functions in the bss. If calling a function, we might (or might not)
    // have its address, keep it in mind and later update all locations

    return bp;
}

void bytecode_generator::initializeVariablesInScope(Scope * scope)
{
    for (auto decl : scope->decls) {
        initializeVariable(decl);
    }
}

void bytecode_generator::initializeVariable(VariableDeclarationAST * decl)
{
    if (!decl->definition) return; // nothing to do for this var
    // @TODO: initialize it to 0 by default
    
    if (decl->definition->ast_type == AST_FUNCTION_DEFINITION) {
        auto fundef = (FunctionDefinitionAST *)decl->definition;
        if (fundef->declaration->isForeign) return; // foreign functions just get called

        // assert(false);
        return;
    }
    // @TODO: we do not support function pointers yet

    // ok, it is an expression (operation, literal (num, string) )
    // or god forbid, another variable (which means load its value
    s16 mark = program->reg_mark();
    s16 reg = program->reserve_register();
    // What if we need more registers? well, for now we allocate extra and assume the best
    if (decl->specified_type->size_in_bits > 64) {
        s64 bits = decl->specified_type->size_in_bits;
        bits -= 64;
        do {
            program->reserve_register();
            bits -= 64;
        } while (bits > 0);
    }
    computeExpressionIntoRegister((ExpressionAST *)decl->definition, reg);
    assert(decl->scope->parent == nullptr);
    BCI *bci = create_instruction(BC_STORE_TO_BSS_PLUS_CONSTANT, reg, -1, decl->bc_mem_offset);
    bci->op_size = truncate_op_size(decl->specified_type->size_in_bits);
    issue_instruction(bci);
    if (decl->specified_type->size_in_bits > 64) {
        s64 bits = decl->specified_type->size_in_bits;
        u64 offset = decl->bc_mem_offset + 8;
        bits -= 64;
        reg++;
        do {
            BCI *bci = create_instruction(BC_STORE_TO_BSS_PLUS_CONSTANT, reg, -1, offset);
            issue_instruction(bci);
            bci->op_size = truncate_op_size(bits);
            reg++;
            bits -= 64;
            offset += 8;
        } while (bits > 0);
    }

    program->pop_mark(mark);
}

void bytecode_generator::computeExpressionIntoRegister(ExpressionAST * expr, s16 reg)
{
    switch (expr->ast_type) {
    case AST_LITERAL: {
        auto lit = (LiteralAST *)expr;

        switch (lit->typeAST.basic_type) {
        case BASIC_TYPE_INTEGER: {
            u64 val;
            if (lit->typeAST.isSigned) val = straight_convert(lit->_s64);
            else val = lit->_u64;
            BCI *bci = create_instruction(BC_LOAD_BIG_CONSTANT_TO_REG, -1, reg, val);
            bci->op_size = lit->typeAST.size_in_bits / 8;
            issue_instruction(bci);
            break;
        }
        case BASIC_TYPE_STRING: {
            // Hacky version, the string data will use the pointer in the AST... 
            // @TODO: think of a better version to handle strings, such as:
            /*
               - Strings in some data segment (all the ones in the program?)
               - allocate memory for all the strings, malloc style
               - Inline the strings (if known size), right after the pointer or such
               - Are strings mutable? How does allocation work? Do we care?
            */
            u64 val = straight_convert(lit->str);
            BCI *bci = create_instruction(BC_LOAD_BIG_CONSTANT_TO_REG, -1, reg, val);
            bci->op_size = 8;
            issue_instruction(bci);
            val = strlen(lit->str);
            bci = create_instruction(BC_LOAD_BIG_CONSTANT_TO_REG, -1, reg+1, val);
            bci->op_size = 8;
            issue_instruction(bci);
            break;
        }
        default:
            assert(false);
                                 // @TODO: add here float, bool... 
        }
        break;
    }
    case AST_IDENTIFIER: {
        auto id = (IdentifierAST *)expr;
        break;
    }
    case AST_UNARY_OPERATION: {
        auto unop = (UnaryOperationAST *)expr;
        break;
    }
    case AST_BINARY_OPERATION: {
        auto binop = (BinaryOperationAST *)expr;
        break;
    }
    case AST_ASSIGNMENT: {
        assert(!"Assignment expression bytecode not implemented");
        break;
    }
    default:
        assert(!"Unknown expression AST for bytecode");
    }
}

void bc_base_memory::initMem(u8 * basemem, u64 bss_size, u64 stack_size)
{
    mem = basemem;
    alloc_size = bss_size + stack_size;
    used_size = 0;
    stack_base = mem + bss_size;
    stack_index = 0;
    this->stack_size = stack_size;
}
