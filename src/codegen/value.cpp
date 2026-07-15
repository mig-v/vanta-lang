#include "codegen/value.h"

bool operator==(const Value& lhs, const Value& rhs)
{
	if (lhs.kind != rhs.kind) 
		return false;

	switch (lhs.kind)
	{
		case ValueKind::VALUE_INT:		    return std::get<int64_t>(lhs.data) == std::get<int64_t>(rhs.data);
		case ValueKind::VALUE_FLOAT:        return std::get<double>(lhs.data) == std::get<double>(rhs.data);
		case ValueKind::VALUE_BOOL:         return std::get<bool>(lhs.data) == std::get<bool>(rhs.data);
		case ValueKind::VALUE_STRING:       return std::get<std::string>(lhs.data) == std::get<std::string>(rhs.data);
		case ValueKind::VALUE_NULL:         return true;
		case ValueKind::VALUE_NATIVE_FN:    return std::get<NativeFn>(lhs.data) == std::get<NativeFn>(rhs.data);
		case ValueKind::VALUE_ARR:          return std::get<Array*>(lhs.data) == std::get<Array*>(rhs.data);
		case ValueKind::VALUE_FILE:         return std::get<File*>(lhs.data) == std::get<File*>(rhs.data);
		case ValueKind::VALUE_INSTANCE:     return std::get<Instance*>(lhs.data) == std::get<Instance*>(rhs.data);
		case ValueKind::VALUE_MODULE:       return std::get<Module*>(lhs.data) == std::get<Module*>(rhs.data);
		case ValueKind::VALUE_FN:           return std::get<Function*>(lhs.data) == std::get<Function*>(rhs.data);
		case ValueKind::VALUE_BOUND_METHOD: return std::get<BoundMethod*>(lhs.data) == std::get<BoundMethod*>(rhs.data);
		default: throw std::runtime_error("ValueKind not implemented in operator== for Value");
	}
}

// used to hash Value's when using an unordered map of Value's
size_t std::hash<Value>::operator()(const Value& val) const
{
	switch (val.kind)
	{
		case ValueKind::VALUE_INT:		    return std::hash<int64_t>{}(std::get<int64_t>(val.data));
		case ValueKind::VALUE_FLOAT:        return std::hash<double>{}(std::get<double>(val.data));
		case ValueKind::VALUE_BOOL:         return std::hash<bool>{}(std::get<bool>(val.data));
		case ValueKind::VALUE_STRING:       return std::hash<std::string>{}(std::get<std::string>(val.data));
		case ValueKind::VALUE_NATIVE_FN:    return std::hash<NativeFn>{}(std::get<NativeFn>(val.data));
		case ValueKind::VALUE_ARR:          return std::hash<Array*>{}(std::get<Array*>(val.data));
		case ValueKind::VALUE_FILE:         return std::hash<File*>{}(std::get<File*>(val.data));
		case ValueKind::VALUE_INSTANCE:     return std::hash<Instance*>{}(std::get<Instance*>(val.data));
		case ValueKind::VALUE_BOUND_METHOD: return std::hash<BoundMethod*>{}(std::get<BoundMethod*>(val.data));
		case ValueKind::VALUE_MODULE:       return std::hash<Module*>{}(std::get<Module*>(val.data));
		case ValueKind::VALUE_FN:           return std::hash<Function*>{}(std::get<Function*>(val.data));
		case ValueKind::VALUE_NULL:         return 0;
		default: throw std::runtime_error("ValueKind not implemented in std::hash<Value>");
	}
}