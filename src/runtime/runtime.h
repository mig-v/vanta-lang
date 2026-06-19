#pragma once

#include "codegen/chunk.h"
#include "runtime/vm.h"


class Runtime
{
public:
	Runtime();
	~Runtime();

	void execute(Module* module);
private:
	VM* vm;
};