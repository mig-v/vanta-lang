#pragma once

#include "codegen/value.h"

namespace Builtins
{
	// built in free floating methods
	Value print(ArgList argList, NativeFnError& ctx);
	Value len(ArgList argList, NativeFnError& ctx);

	// array methods
	Value array_add(Value& object, ArgList argList, NativeFnError& ctx);
	Value array_pop(Value& object, ArgList argList, NativeFnError& ctx);
}