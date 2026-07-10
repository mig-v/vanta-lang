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
	Value file_read(Value& object, ArgList argList, NativeFnCtx& ctx);			// read entire file contents into a string, and return it
	Value file_read_line(Value& object, ArgList argList, NativeFnCtx& ctx);	    // read a line from file into a string, removing the '\n' character
	Value file_seek(Value& object, ArgList argList, NativeFnCtx& ctx);
	Value file_eof(Value& object, ArgList argList, NativeFnCtx& ctx);			// returns whether the file is at the end
}