#pragma once

#include <stdint.h>

enum class Opcode : uint8_t
{
	LOAD_CONST,		// 2 byte operand <const_index>, pushes chunk.constants[const_index] onto the stack

	STORE_GLOBAL,	// 2 byte operand <slot>, pops top of stack and stores into module.globals[slot]
	LOAD_GLOBAL,	// 2 byte operand <slot>, pushes module.globals[slot] onto the stack
	STORE_LOCAL,	// 2 byte operand <slot>, pops top of stack and stores into stack[frame_base + slot]
	LOAD_LOCAL,		// 2 byte operand <slot>, pushes stack[frame_base + slot] onto the stack

	RETURN,			// no operand, returns from function and restores stack state / ip, return value is pushed onto the stack
	NULL_RETURN,	// no operand, used when no return value is used for explicitness

	// binary ops, all work in the same way: no operands, pops the top two values on stack, performs their binary operation on them, pushes result
	// for ops where order matters, the rhs is always at the top of the stack the stack would be stack[..., 1, 2] for the expression 1 << 2
	// where the rhs is retrieved from the first POP, and the lhs the second POP
	ADD,			
	SUB,			
	MUL,			
	DIV,			
	MOD,			
	POW,			
	LT,				
	LTE,
	GT,
	GTE,
	EQ,
	NEQ,
	AND,
	OR,
	XOR,
	L_SHIFT,
	R_SHIFT,

	// unary ops, all work the same way, no operands, pop the top value and apply the unary operation, push result back onto stack
	NOT,
	NEG,
	LOGICAL_NOT,

	// conditional jmps, 2 byte operand <jmp_target>
	// peeks at the top of the stack and jumps to <jmp_target> if true (for jmp_if_true) and jumps if false (for jmp_if_false), doesnt POP
	JMP_IF_TRUE,
	JMP_IF_FALSE,

	// unconditional jump, 2 byte operand <jmp_target>, jumps directly to <jmp_target>
	JMP,

	// pops the top of the stack
	POP,

	// 2 byte operand <argc>
	CALL_FN,

	// no operands, arr and index are on the stack
	ARRAY_LOAD,
	ARRAY_STORE,

	// 2 byte operand <fieldIndex>, used to index into the chunks constant table to get the field name, which is then used to 
	// get the slot for the field via the instances class decl reference field table
	LOAD_FIELD,
	STORE_FIELD,

	EXIT
};