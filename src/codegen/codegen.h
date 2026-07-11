#pragma once

#include <vector>
#include <string>

#include "core/compilation_unit.h"
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

	CompiledModule* compile(const std::vector<ASTNode*>& ast, PipelineContext* ctx, const std::string& filepath, std::vector<CompilationUnit>* dependencies);
private:
	void collect_global_symbols(const std::vector<ASTNode*>& ast, const std::string& filepath);
	void compile_node(ASTNode* node);
	void compile_scoped_node(ASTNode* body);
	IterContext compile_iterable_node(ASTNode* node, uint16_t iteratorSlot);

	void emit_opcode(Opcode opcode, int sourceLine);
	uint16_t emit_operand(uint16_t operand, int sourceLine);	// returns the bytecode address of where <operand> was written
	void emit_store_lhs(ASTNode* lhs);
	void emit_load_lhs(ASTNode* lhs);
	void emit_store_for_entry(EnvEntry entry, int sourceLine);
	void emit_load_for_entry(EnvEntry entry, int sourceLine);
	void emit_compound_assignment_op(TokenKind op, int sourceLine);
	void emit_binary_op(TokenKind op, int sourceLine);
	void emit_unary_op(TokenKind op, int sourceLine);
	void emit_iterable_condition_check(IterContext& iterCtx, int sourceLine);
	void emit_iterable_increment(IterContext& iterCtx, int sourceLine);
	void emit_function(ASTNode* node);
	void emit_method(ASTNode* node);
	void emit_field_decl(ASTNode* node);
	void emit_var_decl(ASTNode* node);
	void emit_implicit_null_return();
	void emit_local_class_instantiation(ASTNode* node);
	void emit_module_class_instantiation(ASTNode* node);

	void patch_loop_context(LoopContext& loopCtx);
	void patch_jump(uint16_t address);

	void register_built_ins();
	void register_built_in_fn(const std::string& name, const Value& nativeFn);

	Function* make_fn(const std::string& name, uint16_t argc, uint16_t localsCount);

	void add_global_at_slot(const std::string& identifier, const Value& value, uint16_t slot);
	uint16_t add_constant_to_chunk(const Value& value);
	Chunk* get_current_chunk();

	bool class_declared_in_module(const std::string& className);
	ClassDecl* find_class_with_name(const std::string& name);
	//ASTEnumDecl* find_enum_with_name(const std::string& name);
	int get_enum_member_by_name(ASTEnumDecl* decl, const std::string memberName);

	int alloc_slot(const std::string& identifier);
	int get_last_line_in_chunk();

	Environment env;
	PipelineContext* ctx;
	CompiledModule* module;
	Function* currentFn;
	ClassDecl* currentClass;
	bool classDepth;
	bool inUserFn;
	bool inConstructor;
	std::vector<LoopContext> loopStack;
	std::vector<CompilationUnit>* dependencies;
};