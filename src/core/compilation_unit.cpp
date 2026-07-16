#include <iostream>
#include <filesystem>

#include "utils/debug_utils.h"
#include "core/compilation_unit.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantics/import_resolver.h"
#include "semantics/semantic_analyzer.h"
#include "codegen/codegen.h"
#include "runtime/runtime.h"

std::string CompilationUnit::get_directory(const std::string& filepath)
{
	auto dir = std::filesystem::path(filepath).parent_path();
	return dir.empty() ? "." : dir.string();
}

std::string CompilationUnit::module_name_from_path(const std::string& filepath)
{
	return std::filesystem::path(filepath).stem().string();
}

bool CompilationUnit::run_pipeline(const std::string& filepath, PipelineContext& ctx, std::unordered_set<std::string>& inProgressImports, std::unordered_map<std::string, CompilationUnit*>& unitCache)
{
	if (!std::filesystem::exists(filepath))
		ctx.reporter.submit_diagnostic({ Phase::Semantic, "input file not found: " + filepath, 0, 0 });

	//std::cout << "running pipeline on " << filepath << std::endl;
	// we can catch errors a bit earlier if we begin by checking if there are any since we may be doing imports and recursively compiling files
	if (ctx.reporter.has_errors()) { return false; }

	// normalize the filepath and insert this file into the in progress set so we can detect circular import errors
	this->filepath = filepath;

	if (inProgressImports.count(this->filepath))
	{
		ctx.reporter.submit_diagnostic({ Phase::Semantic, "circular import detected for file: " + this->filepath, 0, 0 });
		return false;
	}

	inProgressImports.insert(this->filepath);

	// stack initialize the full compilation pipeline
	Lexer lexer;
	Parser parser;
	ImportResolver importResolver;
	SemanticAnalyzer semanticAnalyzer;
	Codegen codegen;
	Runtime runtime;

	// lex the file, we dont need to check for errors since the lexer will not submit errors, just emit INVALID tokens if it needs to
	if (!lexer.lex_file(this->filepath))
		return false;

	// parse and check for errors
	parser.parse_tokens(lexer.get_tokens_ptr(), &ctx);
	if (ctx.reporter.has_errors()) { return false; }

	// run the import resolver, it itself doesn't submit any errors so we dont need to check for any
	imports = importResolver.resolve(parser.get_ast(), ctx, inProgressImports, get_directory(this->filepath), unitCache);
	//std::cout << "imports size for " << filepath << ": " << imports.size() << std::endl;

	// run semantic analysis and check for errors
	semanticAnalyzer.analyze(parser.get_ast(), &ctx);
	if (ctx.reporter.has_errors()) { return false; }

	// run codegen, check for errors, and set the modules name to be used at runtime, can also be an alias like import "math" as "m"
	this->module = codegen.compile(parser.get_ast(), &ctx, this->filepath, &imports);
	this->module->name = this->moduleAlias == "" ? module_name_from_path(filepath) : this->moduleAlias;
	this->module->stemmedPath = module_name_from_path(this->filepath);
	if (ctx.reporter.has_errors()) { return false; }

#ifdef _DEBUG
	//lexer.dump_tokens();
	//std::cout << Utils::serialize_ast(parser.get_ast());
	std::cout << Utils::disassemble_compiled_module(module);
#endif

	// finally remove this file from in progress and return true signifying this file was compiled successfully
	inProgressImports.erase(this->filepath);
	return true;
}