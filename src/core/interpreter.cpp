#include <iostream>

#include "core/interpreter.h"
#include "core/compilation_unit.h"

#include "test_suite/lexer_test_suite.h"
#include "test_suite/parser_test_suite.h"

#include "runtime/runtime.h"

void Interpreter::run(const std::string& mainPath)
{
	std::unordered_set<std::string> inProgressImports;
	Runtime runtime;
	CompilationUnit main;

	main.run_pipeline(mainPath, ctx, inProgressImports);

	if (ctx.reporter.has_errors())
	{
		ctx.reporter.log_diagnostics();
		return;
	}

	compilationUnits.push_back(std::move(main));

#ifdef _DEBUG
	size_t astBytesAlloced = ctx.astArena.get_total_allocated_bytes();
	size_t astBlock = ctx.astArena.get_block_count();
	size_t compilerBytesAlloced = ctx.compilerArena.get_total_allocated_bytes();
	size_t compilerBlockCount = ctx.compilerArena.get_block_count();

	std::cout
		<< "[AST Memory Arena]\n"
		<< "    byted allocated: " << astBytesAlloced << "\n"
		<< "    block count    : " << astBlock << "\n\n"
		<< "[Compiler Memory Arena]\n"
		<< "    byted allocated: " << compilerBytesAlloced << "\n"
		<< "    block count    : " << compilerBlockCount << "\n";
#endif

	// free the ast arena memory when going to the runtime since we no longer need the AST nodes anymore
	ctx.astArena.free_arena();

	runtime.execute(compilationUnits);
}

void Interpreter::run_tests()
{
	LexerTestSuite lexerTestSuite;
	lexerTestSuite.run_suite("Lexer Test Suite");

	ParserTestSuite parserTestSuite;
	//parserTestSuite.regenerate_ast_snapshots();
	parserTestSuite.run_suite("Parser Test Suite");
}