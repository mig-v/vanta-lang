#pragma once

#include "codegen/value.h"

namespace Builtins
{
	// built in free floating methods
	Value print(ArgList argList, NativeFnCtx& ctx);
	Value len(ArgList argList, NativeFnCtx& ctx);
	Value open_file(ArgList argList, NativeFnCtx& ctx);
	Value close_file (ArgList argList, NativeFnCtx& ctx);

	// array methods
	Value array_add(Value& object, ArgList argList, NativeFnCtx& ctx);
	Value array_pop(Value& object, ArgList argList, NativeFnCtx& ctx);

	// file methods
	Value file_close(Value& object, ArgList argList, NativeFnCtx& ctx);
	Value file_write(Value& object, ArgList argList, NativeFnCtx& ctx);
	Value file_write_line(Value& object, ArgList argList, NativeFnCtx& ctx);

}