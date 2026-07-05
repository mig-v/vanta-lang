#include <iostream>

#include "core/interpreter.h"

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "usage: vanta <filepath>" << std::endl;
		return 1;
	}

	std::string filepath = argv[1];
	std::cout << "Main filepath: " << filepath << std::endl;
	Interpreter interpreter;
	interpreter.run_tests();
	
	interpreter.run(filepath);

	//Lexer lexer;
	//Parser parser;

	//if (lexer.lex_file(PROJECT_ROOT"/examples/main.va"))
	//{
	//	lexer.dump_tokens();
	//	parser.parse_tokens(lexer.get_tokens_ptr());
	//	std::cout << Utils::serialize_ast(parser.get_ast());
	//}

	//LexerTestSuite lexerTestSuite;
	//lexerTestSuite.run_suite("Lexer Test Suite");

	//ParserTestSuite parserTestSuite;
	//parserTestSuite.run_suite("Parser Test Suite");

	// TODO: make this a command line arg so we only regenerate the snapshots when we want to
	//parserTestSuite.regenerate_ast_snapshots();

	return 0;
}