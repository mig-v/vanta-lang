#include <fstream>
#include <iostream>

#include "lexer/lexer.h"
#include "utils/debug_utils.h"

Lexer::Lexer()
{
	reset();
	fill_keyword_map();
}

void Lexer::reset()
{
	tokens.clear();
	fileContents.str("");
	fileContents.clear();
	line = 1;
	column = 0;
}

void Lexer::fill_keyword_map()
{
	keywords["var"] = TokenKind::TOKEN_VAR;
	keywords["true"] = TokenKind::TOKEN_TRUE;
	keywords["false"] = TokenKind::TOKEN_FALSE;
	keywords["class"] = TokenKind::TOKEN_CLASS;
	keywords["fn"] = TokenKind::TOKEN_FN;
	keywords["return"] = TokenKind::TOKEN_RETURN;
	keywords["continue"] = TokenKind::TOKEN_CONTINUE;
	keywords["break"] = TokenKind::TOKEN_BREAK;
	keywords["if"] = TokenKind::TOKEN_IF;
	keywords["else"] = TokenKind::TOKEN_ELSE;
	keywords["for"] = TokenKind::TOKEN_FOR;
	keywords["while"] = TokenKind::TOKEN_WHILE;
	keywords["or"] = TokenKind::TOKEN_OR;
	keywords["and"] = TokenKind::TOKEN_AND;
	keywords["not"] = TokenKind::TOKEN_NOT;
	keywords["null"] = TokenKind::TOKEN_NULL;
	keywords["in"] = TokenKind::TOKEN_IN;
}

bool Lexer::lex_file(const std::string& filepath)
{
	// make sure the file exists / is able to be opened
	std::ifstream inFile(filepath);
	if (!inFile.is_open())
	{
		std::cout << "Could not open file <" << filepath << ">\n";
		return false;
	}

	// read the file into our stringstream 'fileContents'
	fileContents << inFile.rdbuf();
	fileContents.seekp(fileContents.beg);
	
	// all is good, begin main lexing loop
	while (!eof())
	{
		skip_whitespace();

		char current = peek();

		// numbers can be ints or floats so we need to check if the first character is a digit or a '.' since stuff like .10 is valid
		if (isdigit(current) || (current == '.' && isdigit(look_ahead())))
			lex_number();

		// identifiers can only begin with an alphabet character or an underscore, these cases also handle keywords
		else if (isalpha(current) || current == '_')
			lex_identifier_or_keyword();
		else
		{
			switch (current)
			{
				case '+':
				{
					advance();

					if (!is_multi_character_operator('=', TokenKind::TOKEN_PLUS_EQUALS))
						emit_token(TokenKind::TOKEN_PLUS);

					break;
				}

				case '=':
				{
					advance();

					if (!is_multi_character_operator('=', TokenKind::TOKEN_EQUALITY))
						emit_token(TokenKind::TOKEN_EQUALS);

					break;
				}

				case '-':
				{
					advance();

					if (!is_multi_character_operator('=', TokenKind::TOKEN_MINUS_EQUALS))
						emit_token(TokenKind::TOKEN_MINUS);

					break;
				}

				case '*':
				{
					advance();

					if (!is_multi_character_operator('=', TokenKind::TOKEN_TIMES_EQUALS) && !is_multi_character_operator('*', TokenKind::TOKEN_POWER))
						emit_token(TokenKind::TOKEN_ASTERISK);

					break;
				}

				case '/':
				{
					advance();

					if (!is_multi_character_operator('=', TokenKind::TOKEN_DIVIDE_EQUALS))
						emit_token(TokenKind::TOKEN_FORWARD_SLASH);

					break;
				}

				case '<':
				{
					advance();

					if (!is_multi_character_operator('=', TokenKind::TOKEN_LTE))
						emit_token(TokenKind::TOKEN_LT);

					break;
				}

				case '>':
				{
					advance();

					if (!is_multi_character_operator('=', TokenKind::TOKEN_GTE))
						emit_token(TokenKind::TOKEN_GT);

					break;
				}

				case '!':
				{
					advance();

					// vanta will not have a '!' operator since it'll use the 'not' keyword.
					// this means that when we read a '!' character, if there is not an '=' character right after, it's an error
					// so we can output an invalid token
					if (!is_multi_character_operator('=', TokenKind::TOKEN_NOT_EQUALITY))
						emit_token(TokenKind::TOKEN_INVALID);

					break;
				}

				case '(':
				{
					advance();
					emit_token(TokenKind::TOKEN_L_PAREN);
					break;
				}

				case ')':
				{
					advance();
					emit_token(TokenKind::TOKEN_R_PAREN);
					break;
				}

				case '{':
				{
					advance();
					emit_token(TokenKind::TOKEN_L_BRACE);
					break;
				}

				case '}':
				{
					advance();
					emit_token(TokenKind::TOKEN_R_BRACE);
					break;
				}

				case '[':
				{
					advance();
					emit_token(TokenKind::TOKEN_L_BRACKET);
					break;
				}

				case ']':
				{
					advance();
					emit_token(TokenKind::TOKEN_R_BRACKET);
					break;
				}

				case ',':
				{
					advance();
					emit_token(TokenKind::TOKEN_COMMA);
					break;
				}

				case ';':
				{
					advance();
					emit_token(TokenKind::TOKEN_SEMICOLON);
					break;
				}

				case '.':
				{
					advance();
					emit_token(TokenKind::TOKEN_DOT);
					break;
				}

				case '%':
				{
					advance();
					emit_token(TokenKind::TOKEN_MODULO);
					break;
				}

				case '#':
				{
					advance_until_newline();
					break;
				}

				case '"':
				{
					lex_string();
					break;
				}
			}
		}
	}

	inFile.close();
	emit_token(TokenKind::TOKEN_EOF);
	return true;
}

char Lexer::advance()
{
	char current = fileContents.get();
	column++;
	return current;
}

char Lexer::look_ahead()
{
	advance();
	char lookAhead = peek();
	rewind();
	return lookAhead;
}

void Lexer::rewind()
{
	fileContents.seekg(-1, std::ios::cur);
	column--;
}

char Lexer::peek()
{
	return fileContents.peek();
}

bool Lexer::eof()
{
	return fileContents.eof();
}

bool Lexer::is_multi_character_operator(char expected, TokenKind kindToEmit)
{
	if (peek() == expected)
	{
		advance();
		emit_token(kindToEmit);
		return true;
	}

	return false;
}

void Lexer::skip_whitespace()
{
	char current = peek();

	while (current == ' ' || current == '\n' || current == '\t')
	{
		advance();
		current = peek();
	}
}

void Lexer::lex_number()
{
	// needs to handle integers and floats
	Token token;
	token.line = line;
	token.column = column;
	token.kind = TokenKind::TOKEN_INT_LITERAL;

	// consume all leading digits before a potential '.' since ints and floats share the same basic structure
	// [0-9]* ['.']* [0-9]*, we get rid of the initial digits here and check for a float later
	while (isdigit(peek()))
		token.tokenLiteral += advance();

	if (peek() == '.')
	{
		token.kind = TokenKind::TOKEN_FLOAT_LITERAL;
		token.tokenLiteral += advance();

		while (isdigit(peek()))
			token.tokenLiteral += advance();
	}
	
	tokens.emplace_back(token);
}

void Lexer::lex_identifier_or_keyword()
{
	Token token;
	token.kind = TokenKind::TOKEN_IDENTIFIER;
	token.line = line;
	token.column = column;

	char current = peek();

	// while the string contains digits, alphabet characters, or underscores, keep reading
	while (isalnum(current) || current == '_')
	{
		token.tokenLiteral += advance();
		current = peek();
	}

	// check if the lexeme is a keyword against our keywords hashmap
	if (keywords.find(token.tokenLiteral) != keywords.end())
		token.kind = keywords[token.tokenLiteral];

	tokens.emplace_back(token);
}

void Lexer::lex_string()
{
	// consume the opening '"'
	advance();

	Token token;
	token.column = column;
	token.line = line;
	token.kind = TokenKind::TOKEN_STRING_LITERAL;

	while (peek() != '"')
		token.tokenLiteral += advance();

	// consume the closing '"'
	advance();
	tokens.emplace_back(token);
}

void Lexer::emit_token(TokenKind kind)
{
	Token token;
	token.kind = kind;
	token.line = line;
	token.column = column;
	tokens.emplace_back(token);
}

void Lexer::advance_until_newline()
{
	// advance through the comment until we reach the newline character
	while (peek() != '\n')
		advance();

	// consume the newline character
	advance();
}

void Lexer::dump_tokens()
{
	for (const Token& token : tokens)
	{
		std::cout << Utils::token_kind_to_string(token.kind);

		if (Utils::token_has_valid_lexeme(token.kind))
			std::cout << " <" << token.tokenLiteral << ">";

		std::cout << "\n";
	}
}