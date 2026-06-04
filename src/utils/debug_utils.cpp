#include "debug_utils.h"

namespace Utils
{
	const char* token_kind_to_string(TokenKind kind)
	{
		switch (kind)
		{
			case TokenKind::TOKEN_VAR: return "TOKEN_VAR";
			case TokenKind::TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";

			case TokenKind::TOKEN_EQUALS: return "TOKEN_EQUALS";
			case TokenKind::TOKEN_PLUS_EQUALS: return "TOKEN_PLUS_EQUALS";
			case TokenKind::TOKEN_MINUS_EQUALS: return "TOKEN_MINUS_EQUALS";
			case TokenKind::TOKEN_TIMES_EQUALS: return "TOKEN_TIMES_EQUALS";
			case TokenKind::TOKEN_DIVIDE_EQUALS: return "TOKEN_DIVIDE_EQUALS";
			case TokenKind::TOKEN_MODULO_EQUALS: return "TOKEN_MODULO_EQUALS";
			case TokenKind::TOKEN_BITWISE_AND_EQUALS: return "TOKEN_BITWISE_AND_EQUALS";
			case TokenKind::TOKEN_BITWISE_OR_EQUALS: return "TOKEN_BITWISE_OR_EQUALS";
			case TokenKind::TOKEN_BITWISE_XOR_EQUALS: return "TOKEN_BITWISE_XOR_EQUALS";
			case TokenKind::TOKEN_BITWISE_L_SHIFT_EQUALS: return "TOKEN_BITWISE_L_SHIFT_EQUALS";
			case TokenKind::TOKEN_BITWISE_R_SHIFT_EQUALS: return "TOKEN_BITWISE_R_SHIFT_EQUALS";

			case TokenKind::TOKEN_INT_LITERAL: return "TOKEN_INT_LITERAL";
			case TokenKind::TOKEN_FLOAT_LITERAL: return "TOKEN_FLOAT_LITERAL";
			case TokenKind::TOKEN_STRING_LITERAL: return "TOKEN_STRING_LITERAL";

			case TokenKind::TOKEN_TRUE: return "TOKEN_TRUE";
			case TokenKind::TOKEN_FALSE: return "TOKEN_FALSE";
			case TokenKind::TOKEN_CLASS: return "TOKEN_CLASS";
			case TokenKind::TOKEN_FN: return "TOKEN_FN";
			case TokenKind::TOKEN_RETURN: return "TOKEN_RETURN";
			case TokenKind::TOKEN_CONTINUE: return "TOKEN_CONTINUE";
			case TokenKind::TOKEN_BREAK: return "TOKEN_BREAK";
			case TokenKind::TOKEN_IF: return "TOKEN_IF";
			case TokenKind::TOKEN_ELSE: return "TOKEN_ELSE";
			case TokenKind::TOKEN_FOR: return "TOKEN_FOR";
			case TokenKind::TOKEN_WHILE: return "TOKEN_WHILE";
			case TokenKind::TOKEN_OR: return "TOKEN_OR";
			case TokenKind::TOKEN_AND: return "TOKEN_AND";
			case TokenKind::TOKEN_NOT: return "TOKEN_NOT";
			case TokenKind::TOKEN_IN:  return "TOKEN_IN";
			case TokenKind::TOKEN_NULL: return "TOKEN_NULL";

			case TokenKind::TOKEN_L_PAREN: return "TOKEN_L_PAREN";
			case TokenKind::TOKEN_R_PAREN: return "TOKEN_R_PAREN";
			case TokenKind::TOKEN_L_BRACE: return "TOKEN_L_BRACE";
			case TokenKind::TOKEN_R_BRACE: return "TOKEN_R_BRACE";
			case TokenKind::TOKEN_L_BRACKET: return "TOKEN_L_BRACKET";
			case TokenKind::TOKEN_R_BRACKET: return "TOKEN_R_BRACKET";
			case TokenKind::TOKEN_COMMA: return "TOKEN_COMMA";
			case TokenKind::TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
			case TokenKind::TOKEN_DOT: return "TOKEN_DOT";
			case TokenKind::TOKEN_RANGE: return "TOKEN_RANGE";

			case TokenKind::TOKEN_LT: return "TOKEN_LT";
			case TokenKind::TOKEN_LTE: return "TOKEN_LTE";
			case TokenKind::TOKEN_GT: return "TOKEN_GT";
			case TokenKind::TOKEN_GTE: return "TOKEN_GTE";
			case TokenKind::TOKEN_NOT_EQUALITY: return "TOKEN_NOT_EQUALITY";
			case TokenKind::TOKEN_EQUALITY: return "TOKEN_EQUALITY";

			case TokenKind::TOKEN_PLUS: return "TOKEN_PLUS";
			case TokenKind::TOKEN_MINUS: return "TOKEN_MINUS";
			case TokenKind::TOKEN_ASTERISK: return "TOKEN_ASTERISK";
			case TokenKind::TOKEN_FORWARD_SLASH: return "TOKEN_FORWARD_SLASH";
			case TokenKind::TOKEN_MODULO: return "TOKEN_MODULO";
			case TokenKind::TOKEN_POWER: return "TOKEN_POWER";

			case TokenKind::TOKEN_INVALID: return "TOKEN_INVALID";
			case TokenKind::TOKEN_EOF: return "TOKEN_EOF";

			case TokenKind::TOKEN_BITWISE_AND: return "TOKEN_BITWISE_AND";
			case TokenKind::TOKEN_BITWISE_OR: return "TOKEN_BITWISE_OR";
			case TokenKind::TOKEN_BITWISE_XOR: return "TOKEN_BITWISE_XOR";
			case TokenKind::TOKEN_BITWISE_NOT: return "TOKEN_BITWISE_NOT";
			case TokenKind::TOKEN_BITWISE_L_SHIFT: return "TOKEN_BITWISE_L_SHIFT";
			case TokenKind::TOKEN_BITWISE_R_SHIFT: return "TOKEN_BITWISE_R_SHIFT";

			default: return "UNIMPLEMENTED_TOKEN_KIND";
		}
	}

	const char* token_op_to_string(TokenKind op)
	{
		switch (op)
		{
			case TokenKind::TOKEN_EQUALS: return "=";
			case TokenKind::TOKEN_PLUS_EQUALS: return "+=";
			case TokenKind::TOKEN_MINUS_EQUALS: return "-=";
			case TokenKind::TOKEN_TIMES_EQUALS: return "*=";
			case TokenKind::TOKEN_DIVIDE_EQUALS: return "/=";
			case TokenKind::TOKEN_MODULO_EQUALS: return "%=";
			case TokenKind::TOKEN_BITWISE_AND_EQUALS: return "&=";
			case TokenKind::TOKEN_BITWISE_OR_EQUALS: return "|=";
			case TokenKind::TOKEN_BITWISE_XOR_EQUALS: return "^=";
			case TokenKind::TOKEN_BITWISE_L_SHIFT_EQUALS: return "<<=";
			case TokenKind::TOKEN_BITWISE_R_SHIFT_EQUALS: return ">>=";

			case TokenKind::TOKEN_LT: return "<";
			case TokenKind::TOKEN_LTE: return "<=";
			case TokenKind::TOKEN_GT: return ">";
			case TokenKind::TOKEN_GTE: return ">=";
			case TokenKind::TOKEN_NOT_EQUALITY: return "!=";
			case TokenKind::TOKEN_EQUALITY: return "==";

			case TokenKind::TOKEN_PLUS: return "+";
			case TokenKind::TOKEN_MINUS: return "-";
			case TokenKind::TOKEN_ASTERISK: return "*";
			case TokenKind::TOKEN_FORWARD_SLASH: return "/";
			case TokenKind::TOKEN_MODULO: return "%";
			case TokenKind::TOKEN_POWER: return "**";

			case TokenKind::TOKEN_OR: return "or";
			case TokenKind::TOKEN_AND: return "and";
			case TokenKind::TOKEN_NOT: return "not";

			case TokenKind::TOKEN_BITWISE_AND: return "&";
			case TokenKind::TOKEN_BITWISE_OR: return "|";
			case TokenKind::TOKEN_BITWISE_XOR: return "^";
			case TokenKind::TOKEN_BITWISE_NOT: return "~";
			case TokenKind::TOKEN_BITWISE_L_SHIFT: return "<<";
			case TokenKind::TOKEN_BITWISE_R_SHIFT: return ">>";

			default: return "NO_VALID_OPERATOR_FOR_TOKEN";
		}
	}

	bool token_has_valid_lexeme(TokenKind kind)
	{
		switch (kind)
		{
			// these tokens have valid lexemes that can be useful when debugging such as function / variable names and literal values
			case TokenKind::TOKEN_IDENTIFIER: 
			case TokenKind::TOKEN_INT_LITERAL:
			case TokenKind::TOKEN_FLOAT_LITERAL:
			case TokenKind::TOKEN_STRING_LITERAL:
				return true;

			// these tokens dont have valid lexemes since they can be deduced based on their TokenKind. E.G. TOKEN_EQUALS will always be '=', etc
			case TokenKind::TOKEN_VAR:
			case TokenKind::TOKEN_EQUALS:
			case TokenKind::TOKEN_PLUS_EQUALS:
			case TokenKind::TOKEN_MINUS_EQUALS:
			case TokenKind::TOKEN_TIMES_EQUALS:
			case TokenKind::TOKEN_DIVIDE_EQUALS:
			case TokenKind::TOKEN_TRUE:
			case TokenKind::TOKEN_FALSE:
			case TokenKind::TOKEN_CLASS:
			case TokenKind::TOKEN_FN:
			case TokenKind::TOKEN_RETURN:
			case TokenKind::TOKEN_CONTINUE:
			case TokenKind::TOKEN_BREAK:
			case TokenKind::TOKEN_IF:
			case TokenKind::TOKEN_ELSE:
			case TokenKind::TOKEN_FOR:
			case TokenKind::TOKEN_WHILE:
			case TokenKind::TOKEN_OR:
			case TokenKind::TOKEN_AND:
			case TokenKind::TOKEN_NOT:
			case TokenKind::TOKEN_NULL:
			case TokenKind::TOKEN_L_PAREN:
			case TokenKind::TOKEN_R_PAREN:
			case TokenKind::TOKEN_L_BRACE:
			case TokenKind::TOKEN_R_BRACE:
			case TokenKind::TOKEN_L_BRACKET:
			case TokenKind::TOKEN_R_BRACKET:
			case TokenKind::TOKEN_COMMA:
			case TokenKind::TOKEN_SEMICOLON:
			case TokenKind::TOKEN_DOT:
			case TokenKind::TOKEN_RANGE:
			case TokenKind::TOKEN_LT:
			case TokenKind::TOKEN_LTE:
			case TokenKind::TOKEN_GT:
			case TokenKind::TOKEN_GTE:
			case TokenKind::TOKEN_NOT_EQUALITY:
			case TokenKind::TOKEN_EQUALITY:
			case TokenKind::TOKEN_PLUS:
			case TokenKind::TOKEN_MINUS:
			case TokenKind::TOKEN_ASTERISK:
			case TokenKind::TOKEN_FORWARD_SLASH:
			case TokenKind::TOKEN_MODULO:
			case TokenKind::TOKEN_POWER:
			case TokenKind::TOKEN_INVALID:
			case TokenKind::TOKEN_EOF:
				return false;

			default: 
				return false;
		}
	}

	const char* ast_kind_to_string(ASTKind kind)
	{
		switch (kind)
		{
			case ASTKind::AST_INT_LITERAL: return "AST_INT_LITERAL";
			case ASTKind::AST_FLOAT_LITERAL: return "AST_FLOAT_LITERAL";
			case ASTKind::AST_STRING_LITERAL: return "AST_STRING_LITERAL";
			case ASTKind::AST_BOOL_LITERAL: return "AST_BOOL_LITERAL";
			case ASTKind::AST_VAR_DECL: return "AST_VAR_DECL";
			case ASTKind::AST_FN_DECL: return "AST_FN_DECL";
			case ASTKind::AST_BLOCK: return "AST_BLOCK";
			case ASTKind::AST_RETURN: return "AST_RETURN";
			case ASTKind::AST_IF: return "AST_IF";
			case ASTKind::AST_WHILE: return "AST_WHILE";
			case ASTKind::AST_FOR: return "AST_FOR";
			case ASTKind::AST_RANGE: return "AST_RANGE";
			case ASTKind::AST_CLASS_DECL: return "AST_CLASS_DECL";
			case ASTKind::AST_NULL: return "AST_NULL";
			case ASTKind::AST_IDENTIFIER: return "AST_IDENTIFIER";
			case ASTKind::AST_ASSIGNMENT: return "AST_ASSIGNMENT";
			case ASTKind::AST_LOGICAL_EXPR: return "AST_LOGICAL_EXPR";
			case ASTKind::AST_BINARY_EXPR: return "AST_BINARY_EXPR";
			case ASTKind::AST_UNARY_EXPR: return "AST_UNARY_EXPR";
			case ASTKind::AST_FN_CALL: return "AST_FN_CALL";
			case ASTKind::AST_ARRAY_ACCESS: return "AST_ARRAY_ACCESS";
			case ASTKind::AST_FIELD_ACCESS: return "AST_FIELD_ACCESS";
			case ASTKind::AST_BREAK: return "AST_BREAK";
			case ASTKind::AST_CONTINUE: return "AST_CONTINUE";
			case ASTKind::AST_THIS: return "AST_THIS";

			default: return "UNIMPLEMENTED_AST_KIND";
		}
	}

	std::string serialize_ast(const std::vector<ASTNode*>& ast)
	{
		std::ostringstream oss{};

		for (ASTNode* node : ast)
			serialize_ast_node(node, 0, oss, false);

		return oss.str();
	}

	void serialize_ast_node(ASTNode* node, int depth, std::ostringstream& oss, bool isField)
	{
		std::string indent(4 * depth, ' ');
		std::string fieldIndent(4 * (depth + 1), ' ');

		if (!isField)
			oss << indent;

		// print the ast nodes data and recurse into any children the node may have
		switch (node->kind)
		{
			case ASTKind::AST_INT_LITERAL:
				oss << "<" << ast_kind_to_string(node->kind) << ">";
				oss  << " { value: " << std::get<ASTIntLiteral>(node->data).value << " }\n";
				break;
			case ASTKind::AST_FLOAT_LITERAL:
				oss << "<" << ast_kind_to_string(node->kind) << ">";
				oss  << " { value: " << std::get<ASTFloatLiteral>(node->data).value << " }\n";
				break;
			case ASTKind::AST_STRING_LITERAL:
				oss << "<" << ast_kind_to_string(node->kind) << ">";
				oss  << " { value: " << std::get<ASTStringLiteral>(node->data).value << " }\n";
				break;
			case ASTKind::AST_BOOL_LITERAL:
				oss << "<" << ast_kind_to_string(node->kind) << ">";
				oss  << " { value: " << std::get<ASTBoolLiteral>(node->data).value << " }\n";
				break;
			case ASTKind::AST_VAR_DECL:
			{
				const ASTVarDecl& data = std::get<ASTVarDecl>(node->data);

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n"
					<< fieldIndent << "identifier: " << data.identifier << "\n"
					<< fieldIndent << "initializer: ";

				serialize_ast_node(data.initializer, depth + 1, oss, true);

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_FN_DECL:
			{
				const ASTFnDecl& data = std::get<ASTFnDecl>(node->data);

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n"
					<< fieldIndent << "fn_name: " << data.identifier << "\n";

				oss << fieldIndent << "params: (";
				if (data.params.size() > 0)
				{
					for (size_t i = 0; i < data.params.size() - 1; i++)
						oss << data.params[i] << ", ";

					oss << data.params[data.params.size() - 1];
				}
				oss << ")\n";

				oss << fieldIndent << "body: ";
				serialize_ast_node(data.body, depth + 1, oss, true);

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_BLOCK:
			{
				const std::vector<ASTNode*>& statements = std::get<ASTBlock>(node->data).statements;

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n"
					<< fieldIndent << "statements:\n";

				for (ASTNode* stmt : statements)
				{
					oss << fieldIndent;
					serialize_ast_node(stmt, depth + 1, oss, true);
				}

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_RETURN:
			{
				const ASTReturn& data = std::get<ASTReturn>(node->data);
				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n"
					<< fieldIndent << "return: ";

				if (data.returnExpr)
					serialize_ast_node(data.returnExpr, depth + 1, oss, true);
				else
					oss << "(implicit null)\n";

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_IF:
			{
				const ASTIf& data = std::get<ASTIf>(node->data);

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n"
					<< fieldIndent << "condition: ";

				serialize_ast_node(data.condition, depth + 1, oss, true);
				oss << fieldIndent << "true branch: ";

				serialize_ast_node(data.trueBranch, depth + 1, oss, true);
				oss << fieldIndent << "false branch: ";

				if (data.falseBranch)
					serialize_ast_node(data.falseBranch, depth + 1, oss, true);
				else oss << " (no branch)\n";
				
				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_WHILE:
			{
				const ASTWhile& data = std::get<ASTWhile>(node->data);

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n"
					<< fieldIndent << "condition: ";

				serialize_ast_node(data.condition, depth + 1, oss, true);
				oss << fieldIndent << "body: ";
				serialize_ast_node(data.body, depth + 1, oss, true);

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_FOR:
			{
				const ASTFor& data = std::get<ASTFor>(node->data);

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n"
					<< fieldIndent << "iterator: " << data.iterator << "\n";

				oss << fieldIndent << "iterable: ";
				serialize_ast_node(data.iterable, depth + 1, oss, true);
				oss << fieldIndent << "body: ";
				serialize_ast_node(data.body, depth + 1, oss, true);
				
				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_RANGE:
			{
				const ASTRange& data = std::get<ASTRange>(node->data);

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n"
					<< fieldIndent << "start: ";

				serialize_ast_node(data.start, depth + 1, oss, true);
				oss << fieldIndent << "end: ";
				serialize_ast_node(data.end, depth + 1, oss, true);

				oss << fieldIndent << "step: ";
				if (data.step)
					serialize_ast_node(data.step, depth + 1, oss, true);
				else oss << " (no step)\n";

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_CLASS_DECL:
			{
				const ASTClassDecl& data = std::get<ASTClassDecl>(node->data);

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n"
					<< fieldIndent << "class_name: " << data.identifier << "\n";

				oss << fieldIndent << "members:\n";

				for (ASTNode* member : data.members)
					serialize_ast_node(member, depth + 1, oss, false);
				
				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_NULL:
			{
				oss << "<" << ast_kind_to_string(node->kind) << ">";
				oss << " { (null) }\n";
				break;
			}
			case ASTKind::AST_IDENTIFIER:
			{
				oss << "<" << ast_kind_to_string(node->kind) << ">";
				oss << " { identifier: " << std::get<ASTIdentifier>(node->data).identifier << " }\n";
				break;
			}
			case ASTKind::AST_ASSIGNMENT:
			{
				const ASTAssignment& data = std::get<ASTAssignment>(node->data);

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n";
				oss << fieldIndent << "operator: " << token_op_to_string(data.op) << "\n";
				oss << fieldIndent << "lhs: ";
				serialize_ast_node(data.lhs, depth + 1, oss, true);
				oss << fieldIndent << "rhs: ";
				serialize_ast_node(data.rhs, depth + 1, oss, true);

				oss << indent << "}\n";
				break;

			}
			case ASTKind::AST_LOGICAL_EXPR:
			{
				const ASTLogicalExpr& data = std::get<ASTLogicalExpr>(node->data);

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n";
				oss << fieldIndent << "operator: " << token_op_to_string(data.op) << "\n";
				oss << fieldIndent << "lhs: ";
				serialize_ast_node(data.lhs, depth + 1, oss, true);
				oss << fieldIndent << "rhs: ";
				serialize_ast_node(data.rhs, depth + 1, oss, true);

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_BINARY_EXPR:
			{
				const ASTBinaryExpr& data = std::get<ASTBinaryExpr>(node->data);

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n";
				oss << fieldIndent << "operator: " << token_op_to_string(data.op) << "\n";
				oss << fieldIndent << "lhs: ";
				serialize_ast_node(data.lhs, depth + 1, oss, true);
				oss << fieldIndent << "rhs: ";
				serialize_ast_node(data.rhs, depth + 1, oss, true);

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_UNARY_EXPR:
			{
				const ASTUnaryExpr& data = std::get<ASTUnaryExpr>(node->data);

				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n";
				oss << fieldIndent << "operator: " << token_op_to_string(data.op) << "\n";
				oss << fieldIndent << "expr: ";
				serialize_ast_node(data.expr, depth + 1, oss, true);

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_FN_CALL:
			{
				const ASTFnCall& data = std::get<ASTFnCall>(node->data);
				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n";
				oss << fieldIndent << "callee: ";
				serialize_ast_node(data.callee, depth + 1, oss, true);

				oss << fieldIndent << "args: ";

				if (data.arguments.size() == 0)
					oss << "(none)";

				oss << "\n";
				for (ASTNode* arg : data.arguments)
				{
					oss << fieldIndent;
					serialize_ast_node(arg, depth + 1, oss, true);
				}

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_ARRAY_ACCESS:
			{
				const ASTArrayAccess& data = std::get<ASTArrayAccess>(node->data);
				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n";

				oss << fieldIndent << "array: ";
				serialize_ast_node(data.arr, depth + 1, oss, true);

				oss << fieldIndent << "index: ";
				serialize_ast_node(data.index, depth + 1, oss, true);

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_FIELD_ACCESS:
			{
				const ASTFieldAccess& data = std::get<ASTFieldAccess>(node->data);
				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				oss << indent << "{\n";

				oss << fieldIndent << "object: ";
				serialize_ast_node(data.object, depth + 1, oss, true);
				oss << fieldIndent << "field_name: " << data.field << "\n";

				oss << indent << "}\n";
				break;
			}
			case ASTKind::AST_BREAK:
			{
				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				break;
			}
			case ASTKind::AST_CONTINUE:
			{
				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				break;
			}
			case ASTKind::AST_THIS:
			{
				oss << "<" << ast_kind_to_string(node->kind) << ">\n";
				break;
			}
			default:
				oss << indent << "{ AST PRINT LOGIC NOT IMPLEMENTED }\n";
		}
	}
}