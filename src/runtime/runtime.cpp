#include "runtime/runtime.h"

Runtime::Runtime()
{
	this->vm = nullptr;
}

Runtime::~Runtime()
{
	delete this->vm;
}

void Runtime::execute(Module* module)
{
	this->vm = new VM();
	vm->execute_module(module);
}