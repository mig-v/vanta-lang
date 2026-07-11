#include "utils/value_utils.h"

// todo, decouple files more so I dont have to include codegen here, value.h is particularly bloated
#include "codegen/codegen.h"

namespace Utils
{
	bool is_truthy(const Value& val)
	{
		switch (val.kind)
		{
			case ValueKind::VALUE_INT:      return std::get<int64_t>(val.data) != 0;
			case ValueKind::VALUE_FLOAT:    return std::get<double>(val.data) != 0.0;
			case ValueKind::VALUE_BOOL:     return std::get<bool>(val.data);
			case ValueKind::VALUE_STRING:   return std::get<std::string>(val.data) != "";
			case ValueKind::VALUE_FN:       return std::get<Function*>(val.data) != nullptr;
			case ValueKind::VALUE_INSTANCE: return true;
			case ValueKind::VALUE_NULL:     return false;
		}
	}

	bool is_numeric(const Value& val)
	{
		return val.kind == ValueKind::VALUE_INT || val.kind == ValueKind::VALUE_FLOAT || val.kind == ValueKind::VALUE_BOOL;
	}

	double to_double(const Value& val)
	{
		if (val.kind == ValueKind::VALUE_INT) return static_cast<double>(std::get<int64_t>(val.data));
		if (val.kind == ValueKind::VALUE_FLOAT) return std::get<double>(val.data);
		if (val.kind == ValueKind::VALUE_BOOL) return std::get<bool>(val.data) ? 1.0 : 0.0;
	}

	int64_t to_int(const Value& val)
	{
		if (val.kind == ValueKind::VALUE_INT) return std::get<int64_t>(val.data);
		if (val.kind == ValueKind::VALUE_FLOAT) return static_cast<int64_t>(std::get<double>(val.data));
		if (val.kind == ValueKind::VALUE_BOOL) return std::get<bool>(val.data) ? 1 : 0;
	}

	std::string to_string(const Value& val)
	{
		switch (val.kind)
		{
			case ValueKind::VALUE_INT:       return std::to_string(std::get<int64_t>(val.data));
			case ValueKind::VALUE_FLOAT:     return std::to_string(std::get<double>(val.data));
			case ValueKind::VALUE_BOOL:      return std::get<bool>(val.data) ? "true" : "false";
			case ValueKind::VALUE_STRING:    return std::get<std::string>(val.data);;
			case ValueKind::VALUE_FN:        return "fn " + std::get<Function*>(val.data)->name;
			case ValueKind::VALUE_INSTANCE:
			{
				Instance* instance = std::get<Instance*>(val.data);
				return "class <" + instance->classDecl->name + ">";
			}
			case ValueKind::VALUE_NULL:      return "null";
			case ValueKind::VALUE_NATIVE_FN: return "builtin fn";
			case ValueKind::VALUE_ARR:
			{
				std::string result = "[";

				Array* ptr = std::get<Array*>(val.data);

				for (size_t i = 0; i < ptr->arr.size(); i++)
				{
					result += to_string(ptr->arr[i]);

					if (i < ptr->arr.size() - 1)
						result += ", ";
				}

				result += "]";
				return result;
			}
			case ValueKind::VALUE_FILE:
			{
				File* filePtr = std::get<File*>(val.data);
				return filePtr->path;
			}
			case ValueKind::VALUE_DICT:
			{
				std::string result = "{\n";
				Dict* dictPtr = std::get<Dict*>(val.data);

				for (const auto& pair : dictPtr->dict)
				{
					result += to_string(pair.first);
					result += " : ";
					result += to_string(pair.second);
					result += "\n";
				}

				result += "}";
				return result;
			}
			case ValueKind::VALUE_MODULE:
			{
				Module* modulePtr = std::get<Module*>(val.data);
				return "module " + modulePtr->name;
			}
		}
	}

	std::string type_name(const Value& val)
	{
		switch (val.kind)
		{
			case ValueKind::VALUE_INT:       return "<int>";
			case ValueKind::VALUE_FLOAT:     return "<float>";
			case ValueKind::VALUE_BOOL:      return "<bool>";
			case ValueKind::VALUE_STRING:    return "<string>";
			case ValueKind::VALUE_FN:        return "<fn>";
			case ValueKind::VALUE_INSTANCE:  return "<instance>";
			case ValueKind::VALUE_NULL:      return "<null>";
			case ValueKind::VALUE_NATIVE_FN: return "<native_fn>";
			case ValueKind::VALUE_ARR:       return "<array>";
			case ValueKind::VALUE_FILE:      return "<file>";
			case ValueKind::VALUE_DICT:      return "<dict>";
			case ValueKind::VALUE_MODULE:    return "<module>";
			default: return "<type not found>";
		}
	}
}