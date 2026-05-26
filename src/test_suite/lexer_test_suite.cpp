#include <iostream>

#include "test_suite/lexer_test_suite.h"

#include "utils/debug_utils.h"

LexerTestSuite::LexerTestSuite()
{
    suite.push_back({ "literals.va",    [this]() { return test_literals(); } });
    suite.push_back({ "keywords.va",    [this]() { return test_keywords(); } });
    suite.push_back({ "identifiers.va", [this]() { return test_identifiers(); } });
    suite.push_back({ "assignment.va",  [this]() { return test_assignment(); } });
    suite.push_back({ "comparison.va",  [this]() { return test_comparison(); } });
    suite.push_back({ "arithmetic.va",  [this]() { return test_arithmetic(); } });
    suite.push_back({ "delimiters.va",  [this]() { return test_delimiters(); } });
}

bool LexerTestSuite::test_literals()
{
    Lexer lexer;
    lexer.lex_file("../tests/lexer/literals.va");
    const std::vector<Token>& tokens = lexer.get_tokens();

    REQUIRE(tokens.size() == 5, "mismatched token count", 5, tokens.size());

    REQUIRE_TOKEN_KIND(tokens[0].kind, TokenKind::TOKEN_INT_LITERAL);
    REQUIRE_TOKEN_LEXEME(tokens[0].tokenLiteral, "10");
    REQUIRE_TOKEN_KIND(tokens[1].kind, TokenKind::TOKEN_FLOAT_LITERAL);
    REQUIRE_TOKEN_LEXEME(tokens[1].tokenLiteral, ".100");
    REQUIRE_TOKEN_KIND(tokens[2].kind, TokenKind::TOKEN_FLOAT_LITERAL);
    REQUIRE_TOKEN_LEXEME(tokens[2].tokenLiteral, "100.0");
    REQUIRE_TOKEN_KIND(tokens[3].kind, TokenKind::TOKEN_STRING_LITERAL);
    REQUIRE_TOKEN_LEXEME(tokens[3].tokenLiteral, "test");
    REQUIRE_TOKEN_KIND(tokens[4].kind, TokenKind::TOKEN_EOF);

    return true;
}

bool LexerTestSuite::test_keywords()
{
    Lexer lexer;
    lexer.lex_file("../tests/lexer/keywords.va");
    const std::vector<Token>& tokens = lexer.get_tokens();

    REQUIRE(tokens.size() == 18, "mismatched token count", 18, tokens.size());

    REQUIRE_TOKEN_KIND(tokens[0].kind, TokenKind::TOKEN_TRUE);
    REQUIRE_TOKEN_KIND(tokens[1].kind, TokenKind::TOKEN_FALSE);
    REQUIRE_TOKEN_KIND(tokens[2].kind, TokenKind::TOKEN_CLASS);
    REQUIRE_TOKEN_KIND(tokens[3].kind, TokenKind::TOKEN_FN);
    REQUIRE_TOKEN_KIND(tokens[4].kind, TokenKind::TOKEN_RETURN);
    REQUIRE_TOKEN_KIND(tokens[5].kind, TokenKind::TOKEN_CONTINUE);
    REQUIRE_TOKEN_KIND(tokens[6].kind, TokenKind::TOKEN_BREAK);
    REQUIRE_TOKEN_KIND(tokens[7].kind, TokenKind::TOKEN_IF);
    REQUIRE_TOKEN_KIND(tokens[8].kind, TokenKind::TOKEN_ELSE);
    REQUIRE_TOKEN_KIND(tokens[9].kind, TokenKind::TOKEN_FOR);
    REQUIRE_TOKEN_KIND(tokens[10].kind, TokenKind::TOKEN_WHILE);
    REQUIRE_TOKEN_KIND(tokens[11].kind, TokenKind::TOKEN_OR);
    REQUIRE_TOKEN_KIND(tokens[12].kind, TokenKind::TOKEN_AND);
    REQUIRE_TOKEN_KIND(tokens[13].kind, TokenKind::TOKEN_NOT);
    REQUIRE_TOKEN_KIND(tokens[14].kind, TokenKind::TOKEN_NULL);
    REQUIRE_TOKEN_KIND(tokens[15].kind, TokenKind::TOKEN_VAR);
    REQUIRE_TOKEN_KIND(tokens[16].kind, TokenKind::TOKEN_IN);
    REQUIRE_TOKEN_KIND(tokens[17].kind, TokenKind::TOKEN_EOF);

    return true;
}

bool LexerTestSuite::test_identifiers()
{
    Lexer lexer;
    lexer.lex_file("../tests/lexer/identifiers.va");
    const std::vector<Token>& tokens = lexer.get_tokens();

    REQUIRE(tokens.size() == 6, "mismatched token count", 6, tokens.size());

    REQUIRE_TOKEN_KIND(tokens[0].kind, TokenKind::TOKEN_IDENTIFIER);
    REQUIRE_TOKEN_LEXEME(tokens[0].tokenLiteral, "foo");
    REQUIRE_TOKEN_KIND(tokens[1].kind, TokenKind::TOKEN_IDENTIFIER);
    REQUIRE_TOKEN_LEXEME(tokens[1].tokenLiteral, "_leadingUnderscore");
    REQUIRE_TOKEN_KIND(tokens[2].kind, TokenKind::TOKEN_IDENTIFIER);
    REQUIRE_TOKEN_LEXEME(tokens[2].tokenLiteral, "snake_case_variable");
    REQUIRE_TOKEN_KIND(tokens[3].kind, TokenKind::TOKEN_IDENTIFIER);
    REQUIRE_TOKEN_LEXEME(tokens[3].tokenLiteral, "numbersInVariable123");
    REQUIRE_TOKEN_KIND(tokens[4].kind, TokenKind::TOKEN_IDENTIFIER);
    REQUIRE_TOKEN_LEXEME(tokens[4].tokenLiteral, "_leading_snake_numbers_3_2");
    REQUIRE_TOKEN_KIND(tokens[5].kind, TokenKind::TOKEN_EOF);

    return true;
}

bool LexerTestSuite::test_assignment()
{
    Lexer lexer;
    lexer.lex_file("../tests/lexer/assignment.va");
    const std::vector<Token>& tokens = lexer.get_tokens();

    REQUIRE(tokens.size() == 6, "mismatched token count", 6, tokens.size());

    REQUIRE_TOKEN_KIND(tokens[0].kind, TokenKind::TOKEN_EQUALS);
    REQUIRE_TOKEN_KIND(tokens[1].kind, TokenKind::TOKEN_PLUS_EQUALS);
    REQUIRE_TOKEN_KIND(tokens[2].kind, TokenKind::TOKEN_MINUS_EQUALS);
    REQUIRE_TOKEN_KIND(tokens[3].kind, TokenKind::TOKEN_TIMES_EQUALS);
    REQUIRE_TOKEN_KIND(tokens[4].kind, TokenKind::TOKEN_DIVIDE_EQUALS);
    REQUIRE_TOKEN_KIND(tokens[5].kind, TokenKind::TOKEN_EOF);

    return true;
}

bool LexerTestSuite::test_comparison()
{
    Lexer lexer;
    lexer.lex_file("../tests/lexer/comparison.va");
    const std::vector<Token>& tokens = lexer.get_tokens();

    REQUIRE(tokens.size() == 7, "mismatched token count", 7, tokens.size());

    REQUIRE_TOKEN_KIND(tokens[0].kind, TokenKind::TOKEN_LT);
    REQUIRE_TOKEN_KIND(tokens[1].kind, TokenKind::TOKEN_LTE);
    REQUIRE_TOKEN_KIND(tokens[2].kind, TokenKind::TOKEN_GT);
    REQUIRE_TOKEN_KIND(tokens[3].kind, TokenKind::TOKEN_GTE);
    REQUIRE_TOKEN_KIND(tokens[4].kind, TokenKind::TOKEN_NOT_EQUALITY);
    REQUIRE_TOKEN_KIND(tokens[5].kind, TokenKind::TOKEN_EQUALITY);
    REQUIRE_TOKEN_KIND(tokens[6].kind, TokenKind::TOKEN_EOF);

    return true;
}

bool LexerTestSuite::test_arithmetic()
{
    Lexer lexer;
    lexer.lex_file("../tests/lexer/arithmetic.va");
    const std::vector<Token>& tokens = lexer.get_tokens();

    REQUIRE(tokens.size() == 7, "mismatched token count", 7, tokens.size());

    REQUIRE_TOKEN_KIND(tokens[0].kind, TokenKind::TOKEN_PLUS);
    REQUIRE_TOKEN_KIND(tokens[1].kind, TokenKind::TOKEN_MINUS);
    REQUIRE_TOKEN_KIND(tokens[2].kind, TokenKind::TOKEN_ASTERISK);
    REQUIRE_TOKEN_KIND(tokens[3].kind, TokenKind::TOKEN_FORWARD_SLASH);
    REQUIRE_TOKEN_KIND(tokens[4].kind, TokenKind::TOKEN_MODULO);
    REQUIRE_TOKEN_KIND(tokens[5].kind, TokenKind::TOKEN_POWER);
    REQUIRE_TOKEN_KIND(tokens[6].kind, TokenKind::TOKEN_EOF);

    return true;
}

bool LexerTestSuite::test_delimiters()
{
    Lexer lexer;
    lexer.lex_file("../tests/lexer/delimiters.va");
    const std::vector<Token>& tokens = lexer.get_tokens();

    REQUIRE(tokens.size() == 10, "mismatched token count", 10, tokens.size());

    REQUIRE_TOKEN_KIND(tokens[0].kind, TokenKind::TOKEN_L_PAREN);
    REQUIRE_TOKEN_KIND(tokens[1].kind, TokenKind::TOKEN_R_PAREN);
    REQUIRE_TOKEN_KIND(tokens[2].kind, TokenKind::TOKEN_L_BRACE);
    REQUIRE_TOKEN_KIND(tokens[3].kind, TokenKind::TOKEN_R_BRACE);
    REQUIRE_TOKEN_KIND(tokens[4].kind, TokenKind::TOKEN_L_BRACKET);
    REQUIRE_TOKEN_KIND(tokens[5].kind, TokenKind::TOKEN_R_BRACKET);
    REQUIRE_TOKEN_KIND(tokens[6].kind, TokenKind::TOKEN_COMMA);
    REQUIRE_TOKEN_KIND(tokens[7].kind, TokenKind::TOKEN_SEMICOLON);
    REQUIRE_TOKEN_KIND(tokens[8].kind, TokenKind::TOKEN_DOT);
    REQUIRE_TOKEN_KIND(tokens[9].kind, TokenKind::TOKEN_EOF);

    return true;
}