#pragma once

#include "core/pipeline_context.h"
#include "core/memory_arena.h"
#include "lexer/token.h"
#include "parser/ast.h"

struct ParseError : public std::exception
{
	const char* what() const noexcept override{ return "parse error"; }
};

class Parser
{
public:
	Parser() = default;
	void parse_tokens(std::vector<Token>* tokens, PipelineContext* ctx);
	
	inline const std::vector<ASTNode*> get_ast() { return ast; }

private:
	ASTNode* statement();
	ASTNode* var_decl();
	ASTNode* fn_decl();
	ASTNode* class_decl();
	ASTNode* if_stmt();
	ASTNode* while_stmt();
	ASTNode* for_stmt();
	ASTNode* for_iterable();
	ASTNode* return_stmt();
	ASTNode* break_stmt();
	ASTNode* continue_stmt();
	ASTNode* expr_stmt();
	ASTNode* expression();
	ASTNode* assignment();
	ASTNode* logical_or();
	ASTNode* logical_and();
	ASTNode* equality();
	ASTNode* bitwise_or();
	ASTNode* bitwise_xor();
	ASTNode* bitwise_and();
	ASTNode* comparison();
	ASTNode* bitwise_shift();
	ASTNode* term();
	ASTNode* factor();
	ASTNode* power();
	ASTNode* unary();
	ASTNode* postfix();
	ASTNode* block();
	ASTNode* primary();

	TokenKind peek();
	Token* advance();
	Token* prev_token();

	bool tokens_left();
	bool peek_is_assignment_op();
	bool peek_is_equality_op();
	bool peek_is_comparison_op();
	bool peek_is_bitwise_shift_op();
	bool peek_is_term_op();
	bool peek_is_factor_op();
	bool peek_is_unary_op();

	Token* assert_current(TokenKind kind, const std::string& errorMsg);
	void throw_error(const std::string& errorMsg);
	void synchronize();

	//MemoryArena arena;
	PipelineContext* ctx;
	std::vector<Token>* tokens;
	std::vector<ASTNode*> ast;
	size_t currentIndex;
	bool inFnDecl;
	bool inClassDecl;
};