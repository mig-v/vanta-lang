#pragma once

#include "core/pipeline_context.h"
#include "core/compilation_unit.h"

class Interpreter
{
public:
	Interpreter() = default;

	void run(const std::string& mainPath);
	void run_tests();
private:
	PipelineContext ctx;
	std::vector<CompilationUnit> compilationUnits;
};