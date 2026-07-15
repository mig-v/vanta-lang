#pragma once

#include <vector>

#include "parser/ast.h"
#include "core/compilation_unit.h"

class ImportResolver
{
public:
	ImportResolver() = default;

	std::vector<CompilationUnit*> resolve(
		const std::vector<ASTNode*>& ast,
		PipelineContext& ctx,
		std::unordered_set<std::string>& inProgressImports,
		const std::string& relativePath,
		std::unordered_map<std::string, CompilationUnit*>& unitCache);
private:
	std::string normalize_path(const std::string& filepath);
};