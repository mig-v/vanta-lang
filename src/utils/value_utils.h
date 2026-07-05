#pragma once

#include "codegen/value.h"

namespace Utils
{
	bool is_truthy(const Value& val);
	bool is_numeric(const Value& val);
	double to_double(const Value& val);
	int64_t to_int(const Value& val);
	std::string to_string(const Value& val);
}