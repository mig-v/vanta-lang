#include <iostream>
#include <sstream>

#include "runtime/built_ins.h"
#include "runtime/gc.h"
#include "utils/value_utils.h"

/* file helper methods NOT in Builtins namespace */
bool file_is_writeable(File* filePtr, NativeFnCtx& ctx)
{
	if (filePtr->closed)
	{
		ctx.error("cannot write to closed file \"" + filePtr->path + "\"");
		return false;
	}

	if (!(filePtr->mode & std::ios::out))
	{
		ctx.error("cannot write to file \"" + filePtr->path + "\" with no write permissions");
		return false;
	}

	return true;
}

bool file_is_readable(File* filePtr, NativeFnCtx& ctx)
{
	if (filePtr->closed)
	{
		ctx.error("cannot read from closed file \"" + filePtr->path + "\"");
		return false;
	}

	if (!(filePtr->mode & std::ios::in))
	{
		ctx.error("cannot read file \"" + filePtr->path + "\" with no read permissions");
		return false;
	}

	return true;
}

namespace Builtins
{
	Value print(ArgList argList, NativeFnCtx& ctx)
	{
		if (argList.args)
		{
			for (const Value& arg : argList)
				std::cout << Utils::to_string(arg);
		}

		std::cout << "\n";
		return Value();
	}

	Value len(ArgList argList, NativeFnCtx& ctx)
	{
		// len expects exactly 1 arg
		if (argList.argc != 1)
		{
			ctx.error("\"len()\" expects 1 arg, got " + std::to_string(argList.argc));
			return Value();
		}

		// check to make sure the arg has a valid length property
		const Value& arg = argList.args[0];
		if (arg.kind == ValueKind::VALUE_ARR)
			return Value(static_cast<int64_t>(std::get<Array*>(arg.data)->arr.size()));
		else if (arg.kind == ValueKind::VALUE_STRING)
			return Value(static_cast<int64_t>(std::get<std::string>(arg.data).size()));

		// if it doesnt, return null and set the ctx's error
		ctx.error("invalid type for 'len()'");
		return Value();
	}

	Value open_file(ArgList argList, NativeFnCtx& ctx)
	{
		// verify argc is 2, and both arguments are strings
		if (argList.argc != 2)
		{
			ctx.error("\"open_file() expects 2 args, (<file_path>, <open_mode>)");
			return Value();
		}

		if (argList.args[0].kind != ValueKind::VALUE_STRING || argList.args[1].kind != ValueKind::VALUE_STRING)
		{
			ctx.error("open_file() args must be strings, expected (<file_path>, <open_mode>)");
			return Value();
		}

		// verify the user passed in a valid mode to open the file with
		const std::string filepath = std::get<std::string>(argList.args[0].data);
		const std::string modeStr = std::get<std::string>(argList.args[1].data);
		std::ios::openmode mode;

		if (modeStr == "r")       mode = std::ios::in;
		else if (modeStr == "w")  mode = std::ios::out | std::ios::trunc;
		else if (modeStr == "a")  mode = std::ios::out | std::ios::app;
		else if (modeStr == "rw") mode = std::ios::in | std::ios::out;
		else
		{
			ctx.error("unsupported open mode for file, expect \"r\", \"w\", \"a\", or \"rw\"");
			return Value();
		}

		// try to open the file and signal an error if the file could not be opened
		std::fstream fileHandle(filepath, mode);
		if (!fileHandle.is_open())
		{
			ctx.error("could not open file at \"" + filepath + "\"");
			return Value();
		}

		// all is good, allocate the file and fill in all members
		File* file = ctx.gc->alloc_object<File>();
		file->file = std::move(fileHandle);
		file->mode = mode;
		file->path = filepath;
		file->closed = false;

		return file;
	}

	Value close_file(ArgList argList, NativeFnCtx& ctx)
	{
		return Value();
	}

	Value array_add(Value& object, ArgList argList, NativeFnCtx& ctx)
	{
		if (argList.argc != 1)
		{
			ctx.error("add() expects 1 arg for arrays, got " + std::to_string(argList.argc));
			return Value();
		}

		Array* arrPtr = std::get<Array*>(object.data);
		Value val = argList.args[0];
		arrPtr->arr.push_back(val);
		return val;
	}

	Value array_pop(Value& object, ArgList argList, NativeFnCtx& ctx)
	{
		if (argList.argc != 0)
		{
			ctx.error("pop() expects 0 args, got " + std::to_string(argList.argc));
			return Value();
		}

		Array* arrPtr = std::get<Array*>(object.data);
		if (arrPtr->arr.size() == 0)
		{
			ctx.error("cannot pop() from empty array");
			return Value();
		}

		Value val = arrPtr->arr.back();
		arrPtr->arr.pop_back();
		return val;
	}

	Value array_clear(Value& object, ArgList argList, NativeFnCtx& ctx)
	{
		if (argList.argc != 0)
		{
			ctx.error("clear() expects 0 args, got " + std::to_string(argList.argc));
			return Value();
		}

		Array* arrPtr = std::get<Array*>(object.data);
		arrPtr->arr.clear();
		return Value();
	}

	Value file_close(Value& object, ArgList argList, NativeFnCtx& ctx)
	{
		if (argList.argc != 0)
		{
			ctx.error("close() expects 0 args, got " + std::to_string(argList.argc));
			return Value();
		}

		File* filePtr = std::get<File*>(object.data);
		filePtr->file.close();
		filePtr->closed = true;
	}

	Value file_write(Value& object, ArgList argList, NativeFnCtx& ctx)
	{
		if (argList.argc != 1)
		{
			ctx.error("write() expects 1 args, got " + std::to_string(argList.argc));
			return Value();
		}

		File* filePtr = std::get<File*>(object.data);

		if (!file_is_writeable(filePtr, ctx))
			return Value();

		// only support converting primitive types and writing them to files, disallow things like functions, arrays, etc.
		const Value& arg = argList.args[0];
		switch (arg.kind)
		{
			case ValueKind::VALUE_STRING:    filePtr->file << std::get<std::string>(arg.data); break;
			case ValueKind::VALUE_INT:       filePtr->file << std::get<int64_t>(arg.data); break;
			case ValueKind::VALUE_FLOAT:     filePtr->file << std::get<double>(arg.data); break;
			case ValueKind::VALUE_BOOL:      filePtr->file << std::get<bool>(arg.data) ? "true" : "false"; break;
			default:
			{
				ctx.error("write() expects strings or primitive types, but got " + Utils::type_name(arg));
				return Value();
			}
		}

		return Value();
	}

	Value file_write_line(Value& object, ArgList argList, NativeFnCtx& ctx)
	{
		if (argList.argc != 1)
		{
			ctx.error("write_line() expects 1 args, got " + std::to_string(argList.argc));
			return Value();
		}

		File* filePtr = std::get<File*>(object.data);

		if (!file_is_writeable(filePtr, ctx))
			return Value();

		// only support converting primitive types and writing them to files, disallow things like functions, arrays, etc.
		const Value& arg = argList.args[0];
		switch (arg.kind)
		{
			case ValueKind::VALUE_STRING:    filePtr->file << std::get<std::string>(arg.data); filePtr->file << "\n"; break;
			case ValueKind::VALUE_INT:       filePtr->file << std::get<int64_t>(arg.data) << "\n"; break;
			case ValueKind::VALUE_FLOAT:     filePtr->file << std::get<double>(arg.data) << "\n"; break;
			case ValueKind::VALUE_BOOL:      filePtr->file << std::get<bool>(arg.data) ? "true\n" : "false\n"; break;
			default:
			{
				ctx.error("write_line() expects strings or primitive types, but got " + Utils::type_name(arg));
				return Value();
			}
		}

		return Value();
	}

	Value file_read(Value& object, ArgList argList, NativeFnCtx& ctx)
	{
		if (argList.argc != 0)
		{
			ctx.error("read() expects 0 args, got " + std::to_string(argList.argc));
			return Value();
		}

		File* filePtr = std::get<File*>(object.data);

		if (!file_is_readable(filePtr, ctx))
			return Value();

		std::ostringstream oss;
		oss << filePtr->file.rdbuf();
		return Value(oss.str());
	}

	Value file_read_line(Value& object, ArgList argList, NativeFnCtx& ctx)
	{
		if (argList.argc != 0)
		{
			ctx.error("read_line() expects 0 args, got " + std::to_string(argList.argc));
			return Value();
		}

		File* filePtr = std::get<File*>(object.data);

		if (!file_is_readable(filePtr, ctx))
			return Value();

		// if file is at eof, return null to signify end of file
		std::string line;
		if (!std::getline(filePtr->file, line))
			return Value();

		// otherwise, return the line as a string with the newline character consumed
		return Value(line);
	}

	Value file_seek(Value& object, ArgList argList, NativeFnCtx& ctx)
	{
		File* filePtr = std::get<File*>(object.data);

		if (argList.argc != 1 || argList.args[0].kind != ValueKind::VALUE_INT)
		{
			ctx.error("seek() expects 1 arg (integer offset)");
			return Value();
		}

		if (filePtr->closed)
		{
			ctx.error("cannot seek() on closed file \"" + filePtr->path + "\"");
			return Value();
		}

		int64_t offset = std::get<int64_t>(argList.args[0].data);
		if (offset < 0)
		{
			ctx.error("seek() expects offset to be a positive integer");
			return Value();
		}

		// need to clear files error bits before setting stream position, this is because reading to end of a file will set the eof bit
		// and attempting to seek to 0 after reading to end will cause issues unless the state is reset
		filePtr->file.clear();
		filePtr->file.seekg(offset);
		filePtr->file.seekp(offset);
	}

	Value file_eof(Value& object, ArgList argList, NativeFnCtx& ctx)
	{
		if (argList.argc != 0)
		{
			ctx.error("eof() expects 0 args, got " + std::to_string(argList.argc));
			return Value();
		}

		File* filePtr = std::get<File*>(object.data);
		if (filePtr->closed)
		{
			ctx.error("cannot call eof() on closed file \"" + filePtr->path + "\"");
			return Value();
		}

		return Value(filePtr->file.eof());

	}
}