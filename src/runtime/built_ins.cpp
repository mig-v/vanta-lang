#include <iostream>

#include "runtime/built_ins.h"

#include "utils/value_utils.h"

namespace Builtins
{
	Value print(ArgList argList, NativeFnError& ctx)
	{
		if (argList.args)
		{
			for (const Value& arg : argList)
				std::cout << Utils::to_string(arg);
		}

		std::cout << "\n";
		return Value();
	}

	Value len(ArgList argList, NativeFnError& ctx)
	{
		// len expects exactly 1 arg
		if (argList.argc != 1)
		{
			ctx.error("\"len\" expects 1 arg, got " + std::to_string(argList.argc));
			return Value();
		}
		
		// check to make sure the arg has a valid length property
		const Value& arg = argList.args[0];
		if (arg.kind == ValueKind::VALUE_ARR)
			return Value(static_cast<int64_t>(std::get<Array*>(arg.data)->arr.size()));
		else if (arg.kind == ValueKind::VALUE_STRING)
			return Value(static_cast<int64_t>(std::get<std::string>(arg.data).size()));

		// if it doesnt, return null and set the ctx's error
		ctx.error("invalid type for 'len'");
		return Value();
	}

	Value array_add(Value& object, ArgList argList, NativeFnError& ctx)
	{
		if (argList.argc != 1)
		{
			ctx.error("\"add\" expects 1 arg for arrays, got " + std::to_string(argList.argc));
			return Value();
		}

		Array* arrPtr = std::get<Array*>(object.data);
		Value val = argList.args[0];
		arrPtr->arr.push_back(val);
		return Value(val);
	}

	Value array_pop(Value& object, ArgList argList, NativeFnError& ctx)
	{
		if (argList.argc != 0)
		{
			ctx.error("\"pop expects 0 args for arrays, got " + std::to_string(argList.argc));
			return Value();
		}

		Array* arrPtr = std::get<Array*>(object.data);
		if (arrPtr->arr.size() == 0)
		{
			ctx.error("cannot pop from empty array");
			return Value();
		}

		Value val = arrPtr->arr.back();
		arrPtr->arr.pop_back();
		return val;
	}
}