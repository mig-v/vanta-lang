#pragma once

#include "lexer/token.h"

namespace Utils
{
	const char* token_kind_to_string(TokenKind kind);
	bool token_has_valid_lexeme(TokenKind kind);
}