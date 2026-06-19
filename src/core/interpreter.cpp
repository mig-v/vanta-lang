#include <iostream>

#include "core/interpreter.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantics/semantic_analyzer.h"
#include "codegen/codegen.h"
#include "runtime/runtime.h"

#include "test_suite/lexer_test_suite.h"
#include "test_suite/parser_test_suite.h"

#include "utils/debug_utils.h"

void Interpreter::run()
{
	Lexer lexer;
	Parser parser;
	SemanticAnalyzer semanticAnalyzer;
	Codegen codegen;
	Runtime runtime;

	if (!lexer.lex_file(PROJECT_ROOT"/examples/main.va"))
		return;

	lexer.dump_tokens();
	parser.parse_tokens(lexer.get_tokens_ptr(), &ctx);
	if (ctx.reporter.has_errors())
	{
		ctx.reporter.log_diagnostics();
		return;
	}

	std::cout << Utils::serialize_ast(parser.get_ast());

	semanticAnalyzer.analyze(parser.get_ast(), &ctx);
	if (ctx.reporter.has_errors())
	{
		ctx.reporter.log_diagnostics();
		return;
	}

	Module* module = codegen.compile(parser.get_ast(), &ctx);
	if (ctx.reporter.has_errors())
	{
		ctx.reporter.log_diagnostics();
		return;
	}
	std::cout << Utils::disassemble_module(module);

	runtime.execute(module);
}

void Interpreter::run_tests()
{
	LexerTestSuite lexerTestSuite;
	lexerTestSuite.run_suite("Lexer Test Suite");

	ParserTestSuite parserTestSuite;
	//parserTestSuite.regenerate_ast_snapshots();
	parserTestSuite.run_suite("Parser Test Suite");
}