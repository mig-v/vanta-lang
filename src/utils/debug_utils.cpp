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

			default: return "UNIMPLEMENTED_TOKEN";
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
}