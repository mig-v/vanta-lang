#include "semantics/import_resolver.h"
#include "codegen/codegen.h"

#include <iostream>
#include <filesystem>

std::string ImportResolver::normalize_path(const std::string& filepath)
{
	return std::filesystem::canonical(filepath).string();
}

std::vector<CompilationUnit*> ImportResolver::resolve(
	const std::vector<ASTNode*>& ast,
	PipelineContext& ctx,
	std::unordered_set<std::string>& inProgressImports,
	const std::string& relativePath,
	std::unordered_map<std::string, CompilationUnit*>& unitCache)
{
	std::vector<CompilationUnit*> units;

	for (ASTNode* node : ast)
	{
		if (node->kind == ASTKind::AST_IMPORT_STMT)
		{
			const ASTImportStmt& data = std::get<ASTImportStmt>(node->data);
			std::string filepath = normalize_path(relativePath + "/" + data.importName + ".va");

			auto it = unitCache.find(filepath);

			// cache hit, this file has already been compiled, point to that CompilationUnit rather than redoing work
			if (it != unitCache.end())
			{
				std::cout << "cache hit for <" << filepath << ">\n";
				units.push_back(it->second);
			}

			// otherwise, allocate a new compilation unit and run the pipeline on it
			else
			{
				CompilationUnit* unit = ctx.compilerArena.alloc<CompilationUnit>();
				unit->moduleAlias = data.alias;
				unitCache[filepath] = unit;
				unit->run_pipeline(filepath, ctx, inProgressImports, unitCache);
				units.push_back(unit);
			}
		}
	}

	return units;
}
