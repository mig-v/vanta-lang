#pragma once

#include <vector>
#include <unordered_set>

#include "core/pipeline_context.h"
#include "parser/ast.h"

class SemanticAnalyzer
{
public:
	SemanticAnalyzer();

	void analyze(const std::vector<ASTNode*>& ast, PipelineContext* ctx);
private:
	void analyze_node(ASTNode* node);
	void check_duplicate_class_member(const std::string& identifier, ASTNode* node);

	PipelineContext* ctx;
	int loopDepth;
	int fnDeclDepth;
	int classDeclDepth;

	// track and fn params and class members to detect when duplicate params / members are made
	std::unordered_set<std::string> fnParams;
	std::unordered_set<std::string> classMembers;
};