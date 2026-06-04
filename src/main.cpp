#include <iostream>

#include "utils/debug_utils.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "test_suite/lexer_test_suite.h"
#include "test_suite/parser_test_suite.h"

int main(int argc, char** argv)
{
	Lexer lexer;
	Parser parser;

	if (lexer.lex_file(PROJECT_ROOT"/examples/main.va"))
	{
		lexer.dump_tokens();
		parser.parse_tokens(lexer.get_tokens_ptr());
		std::cout << Utils::serialize_ast(parser.get_ast());
	}

	LexerTestSuite lexerTestSuite;
	lexerTestSuite.run_suite("Lexer Test Suite");

	ParserTestSuite parserTestSuite;
	parserTestSuite.run_suite("Parser Test Suite");

	// TODO: make this a command line arg so we only regenerate the snapshots when we want to
	//parserTestSuite.regenerate_ast_snapshots();

	return 0;
}