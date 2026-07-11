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
	//fileContents.str("");
	fileContents.clear();
	line = 1;
	column = 0;
	currentIndex = 0;
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
	keywords["this"] = TokenKind::TOKEN_THIS;
	keywords["new"] = TokenKind::TOKEN_NEW;
	keywords["import"] = TokenKind::TOKEN_IMPORT;
	keywords["as"] = TokenKind::TOKEN_AS;
	keywords["enum"] = TokenKind::TOKEN_ENUM;
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

	// read the file into 'fileContents'
	fileContents = std::string((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
	
	// all is good, begin main lexing loop
	while (!eof())
	{
		skip_whitespace();

		if (eof())
			break;

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
					
					if (peek() == '<' && look_ahead() == '=')
					{
						advance();
						advance();
						emit_token(TokenKind::TOKEN_BITWISE_L_SHIFT_EQUALS);
					}

					else if (!is_multi_character_operator('=', TokenKind::TOKEN_LTE) && !is_multi_character_operator('<', TokenKind::TOKEN_BITWISE_L_SHIFT))
						emit_token(TokenKind::TOKEN_LT);

					break;
				}

				case '>':
				{
					advance();

					if (peek() == '>' && look_ahead() == '=')
					{
						advance();
						advance();
						emit_token(TokenKind::TOKEN_BITWISE_R_SHIFT_EQUALS);
					}

					else if (!is_multi_character_operator('=', TokenKind::TOKEN_GTE) && !is_multi_character_operator('>', TokenKind::TOKEN_BITWISE_R_SHIFT))
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

				case ':':
				{
					advance();
					emit_token(TokenKind::TOKEN_COLON);
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

					if (!is_multi_character_operator('.', TokenKind::TOKEN_RANGE))
						emit_token(TokenKind::TOKEN_DOT);

					break;
				}

				case '%':
				{
					advance();
					
					if (!is_multi_character_operator('=', TokenKind::TOKEN_MODULO_EQUALS))
						emit_token(TokenKind::TOKEN_MODULO);

					break;
				}

				case '|':
				{
					advance();
					
					if (!is_multi_character_operator('=', TokenKind::TOKEN_BITWISE_OR_EQUALS))
						emit_token(TokenKind::TOKEN_BITWISE_OR);

					break;
				}

				case '^':
				{
					advance();
					
					if (!is_multi_character_operator('=', TokenKind::TOKEN_BITWISE_XOR_EQUALS))
						emit_token(TokenKind::TOKEN_BITWISE_XOR);

					break;
				}

				case '~':
				{
					advance();
					emit_token(TokenKind::TOKEN_BITWISE_NOT);
					break;
				}

				case '&':
				{
					advance();
					
					if (!is_multi_character_operator('=', TokenKind::TOKEN_BITWISE_AND_EQUALS))
						emit_token(TokenKind::TOKEN_BITWISE_AND);

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

				default:
				{
					std::cout << "lex, default, lex_error called on: " << (int)current << std::endl;
					lex_error();
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
	column++;
	return fileContents[currentIndex++];
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
	currentIndex--;
	column--;
}

char Lexer::peek()
{
	if (eof())
		return '\0';

	return fileContents[currentIndex];
}

char Lexer::peek_next()
{
	if (currentIndex + 1 < fileContents.size())
		return fileContents[currentIndex + 1];
	else
		return '\0';
}

bool Lexer::eof()
{
	return currentIndex >= fileContents.size();
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
		if (current == '\n')
		{
			line++;
			column = 0;
		}

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
		// if we see something like 1.. we think it's a float, but we need to check if there's that second dot there
		// because this can be a range in a for loop like 1..10 wherer we need to emit INT_LITERAL, RANGE, INT_LITERAL
		if (peek_next() == '.')
		{
			tokens.emplace_back(token);
			return;
		}

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

	while (peek() != '"' && !eof())
	{
		char c = advance();

		// check for escape characters
		if (c == '\\' && !eof())
		{
			char escapeChar = advance();
			switch (escapeChar)
			{
				case 'n':  token.tokenLiteral += '\n'; break;
				case 'r':  token.tokenLiteral += '\r'; break;
				case 't':  token.tokenLiteral += '\t'; break;
				case '\\': token.tokenLiteral += '\\'; break;
				case '"':  token.tokenLiteral += '"';  break;
				case '\'': token.tokenLiteral += '\''; break;
				case '0':  token.tokenLiteral += '\0'; break;
			}
		}
		else
		{
			token.tokenLiteral += c;
		}
	}

	// consume the closing '"'
	advance();
	tokens.emplace_back(token);
}

void Lexer::lex_error()
{
	Token token;
	token.column = column;
	token.line = line;
	token.kind = TokenKind::TOKEN_INVALID;

	while (!eof() && peek() != '\n')
		token.tokenLiteral += advance();

	// consume the newline character
	if (!eof())
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
	while (!eof() && peek() != '\n')
		advance();

	// consume the newline character
	if (!eof())
		advance();

	line++;
	column = 0;
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