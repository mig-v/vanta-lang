#pragma once

#include <vector>
#include <sstream>

#include "lexer/token.h"
#include "parser/ast.h"

namespace Utils
{
	const char* token_kind_to_string(TokenKind kind);
	const char* token_op_to_string(TokenKind op);
	bool token_has_valid_lexeme(TokenKind kind);

	const char* ast_kind_to_string(ASTKind kind);
	std::string serialize_ast(const std::vector<ASTNode*>& ast);
	void serialize_ast_node(ASTNode* node, int depth, std::ostringstream& oss, bool isField);
}