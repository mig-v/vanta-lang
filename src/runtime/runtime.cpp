#include "runtime/runtime.h"

#include <iostream>
Runtime::Runtime()
{
	this->vm = nullptr;
}

Runtime::~Runtime()
{
	delete this->vm;
}

void Runtime::execute(std::vector<CompilationUnit>& compilationUnits)
{
	this->vm = new VM();

	// main is guaranteed to be the last compilation unit in the vector
	CompilationUnit& main = compilationUnits.back();
	// recursively initialize all dependencies of the program
	for (CompilationUnit& dependency : main.imports)
	{
		if (!dependency.initialized)
			execute_compilation_unit(dependency, compilationUnits);
	}

	// patch all of mains imported modules after they're initialized
	for (CompilationUnit& dependency : main.imports)
	{
		uint16_t slot = main.module->moduleMap[dependency.module->name];
		main.module->globals[slot] = Value(dependency.runtimeModule);
	}

	// now everything is initialized, execute main
	main.runtimeModule = vm->create_runtime_module(main.module);
	vm->execute_module(main.runtimeModule);
}

void Runtime::execute_compilation_unit(CompilationUnit& unit, std::vector<CompilationUnit>& allUnits)
{
	// for each compilation unit, recurse into each of its imported dependencies and execute them first
	for (CompilationUnit& dependency : unit.imports)
	{
		if (!dependency.initialized)
		{
			// recurse into the deepest dependency
			execute_compilation_unit(dependency, allUnits);

			// run the module, this fills the modules globals table with the correct state of the program
			uint16_t slot = unit.module->moduleMap[dependency.module->name];
			unit.module->globals[slot] = Value(dependency.runtimeModule);
		}
	}

	unit.runtimeModule = vm->create_runtime_module(unit.module);
	vm->initialize_module(unit.runtimeModule);
	unit.initialized = true;
}