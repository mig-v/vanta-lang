#pragma once

#include "core/compilation_unit.h"
#include "codegen/chunk.h"
#include "runtime/vm.h"

class Runtime
{
public:
	Runtime();
	~Runtime();

	void execute(std::vector<CompilationUnit>& compilationUnits);
private:
	bool execute_compilation_unit(CompilationUnit& unit, std::vector<CompilationUnit>& allUnits);

	VM* vm;
};