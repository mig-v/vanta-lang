#include <iostream>

#include "lexer/lexer.h"
#include "test_suite/lexer_test_suite.h"

int main(int argc, char** argv)
{
	Lexer lexer;

	if (lexer.lex_file("../examples/main.va"))
		lexer.dump_tokens();

	LexerTestSuite lexerTestSuite;
	lexerTestSuite.run_suite("Lexer Test Suite");

	return 0;
}