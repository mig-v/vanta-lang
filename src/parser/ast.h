#pragma once

#include <stdint.h>
#include <variant>

#include "lexer/token.h"

struct ASTNode;

enum class ASTKind
{
	AST_INT_LITERAL,
	AST_FLOAT_LITERAL,
	AST_STRING_LITERAL,
	AST_BOOL_LITERAL,
	AST_ARRAY,
	AST_DICT,
	AST_VAR_DECL,
	AST_FN_DECL,
	AST_BLOCK,
	AST_RETURN,
	AST_IF,
	AST_WHILE,
	AST_FOR,
	AST_RANGE,
	AST_CLASS_DECL,
	AST_IDENTIFIER,
	AST_ASSIGNMENT,
	AST_LOGICAL_EXPR,
	AST_BINARY_EXPR,
	AST_UNARY_EXPR,
	AST_FN_CALL,
	AST_ARRAY_ACCESS,
	AST_FIELD_ACCESS,
	AST_BREAK,
	AST_CONTINUE,
	AST_NULL,
	AST_THIS,
	AST_INSTANTIATION,
	AST_EXPR_STMT,
	AST_IMPORT_STMT
};

struct ASTIntLiteral
{
	ASTIntLiteral(int64_t value) : value(value) {}
	int64_t value;
};

struct ASTFloatLiteral
{
	ASTFloatLiteral(double value) : value(value) {}
	double value;
};

struct ASTStringLiteral
{
	ASTStringLiteral(std::string value) : value(value) {}
	std::string value;
};

struct ASTBoolLiteral
{
	ASTBoolLiteral(bool value) : value(value) {}
	bool value;
};

struct ASTArray
{
	ASTArray(std::vector<ASTNode*> arr) : arr(std::move(arr)) {}

	std::vector<ASTNode*> arr;
};

struct ASTDict
{
	ASTDict(std::vector<ASTNode*> keys, std::vector<ASTNode*> vals) : keys(std::move(keys)), vals(std::move(vals)) {}

	std::vector<ASTNode*> keys;
	std::vector<ASTNode*> vals;
};

struct ASTVarDecl
{
	ASTVarDecl(const std::string& identifier, ASTNode* initializer) 
		: identifier(identifier), initializer(initializer) {}

	std::string identifier;
	ASTNode* initializer;
};

struct ASTFnDecl
{
	ASTFnDecl(const std::string& identifier, std::vector<std::string> params, ASTNode* body)
		: identifier(identifier), params(std::move(params)), body(body) {}

	std::string identifier;
	std::vector<std::string> params;
	ASTNode* body;
};

struct ASTBlock
{
	ASTBlock(std::vector<ASTNode*> statements) : statements(std::move(statements)) {}
	std::vector<ASTNode*> statements;
};

struct ASTReturn
{
	ASTReturn(ASTNode* returnExpr) : returnExpr(returnExpr) {}
	ASTNode* returnExpr;
};

struct ASTIf
{
	ASTIf(ASTNode* condition, ASTNode* trueBranch, ASTNode* falseBranch)
		: condition(condition), trueBranch(trueBranch), falseBranch(falseBranch) {}

	ASTNode* condition;
	ASTNode* trueBranch;
	ASTNode* falseBranch;
};

struct ASTWhile
{
	ASTWhile(ASTNode* condition, ASTNode* body) : condition(condition), body(body) {}

	ASTNode* condition;
	ASTNode* body;
};

struct ASTFor
{
	ASTFor(const std::string& iterator, ASTNode* iterable, ASTNode* body)
		: iterator(iterator), iterable(iterable), body(body) {}

	std::string iterator;
	ASTNode* iterable;
	ASTNode* body;
};

struct ASTRange
{
	ASTRange(ASTNode* start, ASTNode* end, ASTNode* step)
		: start(start), end(end), step(step) {}

	ASTNode* start;
	ASTNode* end;
	ASTNode* step;
};

struct ASTClassDecl
{
	ASTClassDecl(const std::string& identifier, std::vector<ASTNode*> members)
		: identifier(identifier), members(std::move(members)) {}

	std::string identifier;
	std::vector<ASTNode*> members;
};

struct ASTIdentifier
{
	ASTIdentifier(const std::string& identifier) : identifier(identifier) {}

	std::string identifier;
};

struct ASTAssignment
{
	ASTAssignment(ASTNode* lhs, TokenKind op, ASTNode* rhs) 
		: lhs(lhs), op(op), rhs(rhs) {}

	ASTNode* lhs;
	TokenKind op;
	ASTNode* rhs;
};

struct ASTLogicalExpr
{
	ASTLogicalExpr(ASTNode* lhs, TokenKind op, ASTNode* rhs) 
		: lhs(lhs), op(op), rhs(rhs) {}

	ASTNode* lhs;
	TokenKind op;
	ASTNode* rhs;
};

struct ASTBinaryExpr
{
	ASTBinaryExpr(ASTNode* lhs, TokenKind op, ASTNode* rhs) 
		: lhs(lhs), op(op), rhs(rhs) {}

	ASTNode* lhs;
	TokenKind op;
	ASTNode* rhs;
};

struct ASTUnaryExpr
{
	ASTUnaryExpr(TokenKind op, ASTNode* expr) : op(op), expr(expr) {}

	TokenKind op;
	ASTNode* expr;
};

struct ASTFnCall
{
	ASTFnCall(ASTNode* callee, std::vector<ASTNode*> arguments)
		: callee(callee), arguments(std::move(arguments)) {}

	ASTNode* callee;
	std::vector<ASTNode*> arguments;
};

struct ASTArrayAccess
{
	ASTArrayAccess(ASTNode* arr, ASTNode* index) : arr(arr), index(index) {}

	ASTNode* arr;
	ASTNode* index;
};

struct ASTFieldAccess
{
	ASTFieldAccess(ASTNode* object, const std::string& field) : object(object), field(field) {}

	ASTNode* object;
	std::string field;
};

struct ASTExprStmt
{
	ASTExprStmt(ASTNode* expr) : expr(expr) {}
	ASTNode* expr;
};

struct ASTInstantiation
{
	ASTInstantiation(std::vector<std::string> path, std::vector<ASTNode*> args) : path(std::move(path)), args(std::move(args)) {}

	// the last element is always the class name, every other element is module access
	std::vector<std::string> path;
	std::vector<ASTNode*> args;
};

struct ASTImportStmt
{
	ASTImportStmt(const std::string& importName, const std::string& alias) : importName(importName), alias(alias) {}

	std::string importName;
	std::string alias;
};

struct ASTBreak {};
struct ASTContinue {};
struct ASTNull {};
struct ASTThis {};

using ASTData = std::variant 
<
	ASTIntLiteral,
	ASTFloatLiteral,
	ASTStringLiteral,
	ASTBoolLiteral,
	ASTArray,
	ASTDict,
	ASTVarDecl,
	ASTFnDecl,
	ASTBlock,
	ASTReturn,
	ASTIf,
	ASTWhile,
	ASTFor,
	ASTRange,
	ASTClassDecl,
	ASTIdentifier,
	ASTAssignment,
	ASTLogicalExpr,
	ASTBinaryExpr,
	ASTUnaryExpr,
	ASTFnCall,
	ASTArrayAccess,
	ASTFieldAccess,
	ASTBreak,
	ASTContinue,
	ASTNull,
	ASTThis,
	ASTInstantiation,
	ASTExprStmt,
	ASTImportStmt
>;

struct ASTNode
{
	ASTNode(ASTKind kind, int line, int column, ASTData data) : kind(kind), line(line), column(column), data(data) {}

	ASTKind kind;
	uint32_t line, column;
	ASTData data;
};