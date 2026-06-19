#pragma once

#include <string>
#include <variant>

// forward declare chunk since Functions need to store their relevant chunks of bytecode which is defined in chunk.h
struct Chunk;
struct ClassDecl;
struct Instance;

struct NullValue
{
	bool operator==(const NullValue&) const { return true; }
};

struct Function
{
	std::string name;
	uint16_t argc;
	uint16_t locals;
	Chunk* chunk;
};

using ValueData = std::variant
<
	int64_t,
	double,
	bool,
	std::string,
	NullValue,
	Function*,
	Instance*
>;

enum class ValueKind
{
	VALUE_INT,
	VALUE_FLOAT,
	VALUE_STRING,
	VALUE_BOOL,
	VALUE_NULL,
	VALUE_FN,
	VALUE_INSTANCE
};

struct Value
{
	Value() = default;
	Value(int64_t val) : kind(ValueKind::VALUE_INT), data(std::in_place_type<int64_t>, val) {}
	Value(double val) : kind(ValueKind::VALUE_FLOAT), data(std::in_place_type<double>, val) {}
	Value(bool val) : kind(ValueKind::VALUE_BOOL), data(std::in_place_type<bool>, val) {}
	Value(const std::string& val) : kind(ValueKind::VALUE_STRING), data(std::in_place_type<std::string>, val) {}
	Value(NullValue val) : kind(ValueKind::VALUE_NULL), data(std::in_place_type<NullValue>) {}


	Value(ValueKind kind, ValueData data) : kind(kind), data(data) {}

	ValueKind kind;
	ValueData data;
};

struct Instance
{
	ClassDecl* classDecl;
	std::vector<Value> fields;
};