#pragma once

#include <string>
#include <vector>
#include <unordered_set>

#include "core/pipeline_context.h"
#include "codegen/value.h"

// forward declare, all necessary compilation phases are included in compilation_unit.cpp
struct CompiledModule;

// class to maintain all necessary metadata related to one source file that was compiled
class CompilationUnit
{
public:
	bool run_pipeline(const std::string& filepath, PipelineContext& ctx, std::unordered_set<std::string>& inProgressImports);

	std::string filepath;
	std::string moduleAlias;
	std::vector<CompilationUnit> imports;
	CompiledModule* module;
	Module* runtimeModule;
	
	bool initialized = false;

private:
	std::string get_directory(const std::string& filepath);
	std::string normalize_path(const std::string& filepath);
	std::string module_name_from_path(const std::string& filepath);
};