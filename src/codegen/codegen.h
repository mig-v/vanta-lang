#pragma once

#include <vector>
#include <string>

#include "core/pipeline_context.h"
#include "parser/ast.h"
#include "codegen/opcode.h"
#include "codegen/value.h"
#include "codegen/environment.h"
#include "codegen/chunk.h"

struct LoopContext
{
	LoopContext(uint16_t startAddress) : startAddress(startAddress) {}

	uint16_t startAddress;
	std::vector<uint16_t> breakJumps;
	std::vector<uint16_t> continueJumps;
};

struct RangeIterContext
{
	int iterSlot;
	int endSlot;
	int stepSlot;
	uint16_t conditionAddress;
	uint16_t endJmpPatch;
};

struct CollectionIterContext
{

};

using IterContext = std::variant<RangeIterContext, CollectionIterContext>;

class Codegen
{
public:
	Codegen();

	Module* compile(const std::vector<ASTNode*>& ast, PipelineContext* ctx);
private:
	void compile_node(ASTNode* node);
	void compile_scoped_node(ASTNode* body);

	IterContext compile_iterable_node(ASTNode* node, uint16_t iteratorSlot);

	void emit_opcode(Opcode opcode);
	uint16_t emit_operand(uint16_t operand);	// returns the bytecode address of where <operand> was written
	void emit_store_lhs(ASTNode* lhs);
	void emit_load_lhs(ASTNode* lhs);
	void emit_store_for_entry(EnvEntry entry);
	void emit_load_for_entry(EnvEntry entry);
	void emit_compound_assignment_op(TokenKind op);
	void emit_binary_op(TokenKind op);
	void emit_unary_op(TokenKind op);
	void emit_iterable_condition_check(IterContext& iterCtx);
	void emit_iterable_increment(IterContext& iterCtx);
	void emit_function(ASTNode* node);
	void emit_method(ASTNode* node);
	void emit_field_decl(ASTNode* node);
	void emit_var_decl(ASTNode* node);

	void patch_loop_context(LoopContext& loopCtx);
	void patch_jump(uint16_t address);

	Function* make_fn(const std::string& name, uint16_t argc, uint16_t localsCount);

	void add_global_at_slot(const Value& value, uint16_t slot);
	uint16_t add_constant_to_chunk(const Value& value);
	Chunk* get_current_chunk();

	bool class_declared_in_module(const std::string& className);

	int alloc_slot(const std::string& identifier);

	Environment env;
	PipelineContext* ctx;
	Module* module;
	Function* currentFn;
	ClassDecl* currentClass;
	bool classDepth;
	std::vector<LoopContext> loopStack;
};