#include "semantics/import_resolver.h"
#include <iostream>
std::vector<CompilationUnit> ImportResolver::resolve(
	const std::vector<ASTNode*>& ast,
	PipelineContext& ctx,
	std::unordered_set<std::string>& inProgressImports,
	const std::string& relativePath)
{
	std::vector<CompilationUnit> units;

	for (ASTNode* node : ast)
	{
		if (node->kind == ASTKind::AST_IMPORT_STMT)
		{
			const ASTImportStmt& data = std::get<ASTImportStmt>(node->data);
			std::string filepath = relativePath + "/" + data.importName + ".va";

			CompilationUnit unit;
			unit.moduleAlias = data.alias;
			unit.run_pipeline(filepath, ctx, inProgressImports);

			// need to make sure all files imported within this compilation unit are merged first before the current compilation unit
			units.insert(units.end(), unit.imports.begin(), unit.imports.end());
			units.push_back(std::move(unit));
		}
	}

	std::cout << "returning compilation units size: " << units.size() << std::endl;
	return units;
}
