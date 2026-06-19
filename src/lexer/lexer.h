#pragma once

#include <unordered_map>
#include <string>
#include <sstream>
#include <stdint.h>

#include "lexer/token.h"

class Lexer
{
public:
	Lexer();

	bool lex_file(const std::string& filepath);

	void dump_tokens();
	void reset();

	inline const std::vector<Token>& get_tokens() { return tokens; }
	inline std::vector<Token>* get_tokens_ptr() { return &tokens; }

private:
	void fill_keyword_map();
	void skip_whitespace();
	void lex_number();
	void lex_identifier_or_keyword();
	void lex_string();
	void lex_error();
	void emit_token(TokenKind kind);
	void advance_until_newline();
	void rewind();

	char peek();
	char peek_next();
	char advance();
	char look_ahead();

	bool eof();
	bool is_multi_character_operator(char expected, TokenKind kindToEmit);

	std::uint32_t line;
	std::uint32_t column;

	std::vector<Token> tokens;
	std::unordered_map<std::string, TokenKind> keywords;
	std::string fileContents;
	std::size_t currentIndex;
};